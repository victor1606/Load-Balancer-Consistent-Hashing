/* Copyright 2021 <Calugaritoiu Ion-Victor> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "linkedlist.h"
#include "hashtable.h"
#include "utils.h"

struct load_balancer {
	linked_list_t *server_list;
};

/*
 * Functii de hashing:
 */
unsigned int
hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int
hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *) a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

/*
 * Functia initializeaza si aloca memorie pentru structura load_balancer:
 */
load_balancer*
init_load_balancer() {
	load_balancer *main_server = malloc(sizeof(load_balancer));
	DIE(!main_server, "load_balancer malloc");

	main_server->server_list = ll_create(sizeof(server_memory));

	return main_server;
}

/*
 * Functia obtine server-ul pe care trebuie stocata perechea cheie-valoare
 * primita ca parametru:
 */
ll_node_t*
find_hashring_spot(load_balancer* main_server,
	char* key, int* server_id) {
	if (main_server->server_list->size > 0) {
		unsigned int hash = (unsigned int)hash_function_key(key);
		*server_id = 1;

		ll_node_t *curr = main_server->server_list->head;
		ll_node_t *server;

		if (((server_memory *)curr->data)->hash > hash) {
			server = main_server->server_list->head;
			*server_id = ((server_memory *)server->data)->server_id;
		} else {
			while (curr->next) {
				if (((server_memory *)curr->data)->hash < hash &&
					((server_memory *)curr->next->data)->hash > hash)
						break;

				curr = curr->next;
				(*server_id)++;
			}

			if ((unsigned int)*server_id == main_server->server_list->size)
				*server_id = 0;

			server = ll_get_nth_node(main_server->server_list, *server_id);
			*server_id = ((server_memory *)server->data)->server_id;
		}
		return server;
	}
	return NULL;
}

/*
 * Functia stocheaza o pereche cheie-valoare pe server-ul potrivit:
 */
void
loader_store(load_balancer* main_server, char* key,
	char* value, int* server_id) {
	ll_node_t *server = find_hashring_spot(main_server, key, server_id);
	server_store((server_memory *)(server->data), key, value);
}

/*
 * Functia obtine valoarea corespunzatoare cheii date ca parametru:
 */
char*
loader_retrieve(load_balancer* main_server, char* key, int* server_id) {
	ll_node_t *server = find_hashring_spot(main_server, key, server_id);
	return server_retrieve((server_memory *)(server->data), key);
}

/*
 * Functia gaseste fiecare cheie stocata pe server-ul primit ca parametru
 * si recalculeaza pozitia sa pe hashring:
 */
void
reassign_keys(load_balancer* main_server, ll_node_t *server) {
	for (int i = 0; i < NR_BUCKETS; ++i) {
		if (((server_memory *)server->data)->ht->buckets[i]->size > 0) {
			ll_node_t *curr = ((server_memory *)server->data)->ht->buckets[i]->head;

			while (curr) {
				int index_server = 0;
				loader_store(main_server, (char *)((info *)curr->data)->key,
					(char *)((info *)curr->data)->value, &index_server);
				curr = curr->next;
			}
		}
	}
}

/*
 * Functia asigura ca perechile cheie-valoare sunt stocate corespunzator
 * in hashring dupa adaugarea unui noi server in lista:
 */
void
check_key_placement(load_balancer* main_server) {
	ll_node_t *temp = main_server->server_list->head;
	hashtable_t *reassigned = ht_create(100, hash_function_key,
		compare_function_ints);
	int place_holder = 0;
	while(temp) {
		int server_id = ((server_memory *)temp->data)->server_id;
		if (!ht_get(reassigned, &server_id)) {
			reassign_keys(main_server, temp);
			ht_put(reassigned, &server_id, sizeof(int),
				&place_holder, sizeof(int));
		}
		temp = temp->next;
	}
	ht_free(reassigned);
}

/*
 * Functia adauga un nou server in lista + cele 2 replici ale sale:
 */
void
loader_add_server(load_balancer* main_server, int server_id) {
	server_memory *server = init_server_memory();
	server->server_id = server_id;

	for (int i = 0; i < 3; ++i) {
		int label = i * 100000 + server_id;
		server->hash = (unsigned int)hash_function_servers(&label);

		int pos = 1;

		if (main_server->server_list->size > 0) {
			ll_node_t *curr = main_server->server_list->head;

			if (((server_memory *)curr->data)->hash >= server->hash) {
				pos = 0;
			} else {
				while (curr->next) {
					if (((server_memory *)curr->data)->hash < server->hash &&
						((server_memory *)curr->next->data)->hash > server->hash)
							break;
					curr = curr->next;
					pos++;
				}
			}
		} else {
			pos = 0;
		}

		ll_add_nth_node(main_server->server_list, pos, server);

		// se verifica plasarea cheilor doar dupa adaugarea ultimii replici:
		if (i == 2)
			check_key_placement(main_server);
	}
	free(server);
}

/*
 * Functia sterge din lista server-ul cu id-ul dat ca parametru:
 */
void
loader_remove_server(load_balancer* main_server, int server_id) {
	ll_node_t *curr_server = main_server->server_list->head;
	int i = 0, count = 0;
	while (curr_server) {
		if (((server_memory *)curr_server->data)->server_id == server_id) {
			count++;

			hashtable_t *table = ((server_memory *)curr_server->data)->ht;

			curr_server = curr_server->next;

			ll_node_t *deleted = ll_remove_nth_node(main_server->server_list, i);

			// dupa ce se sterge si ultima replica corespunzatoare server-ului,
			// toate perechile cheie-valoare apartinand acestuia sunt reassigned:
			if (count == NR_REPLICAS) {
				reassign_keys(main_server, deleted);
				ht_free(table);
			}

			free(deleted->data);
			free(deleted);
			continue;
		}
		i++;
		curr_server = curr_server->next;
	}
}

/*
 * Functia asigura eliberarea completa a memoriei utilizate:
 */
void
free_load_balancer(load_balancer* main_server) {
	ll_node_t *curr = main_server->server_list->head;
	hashtable_t *free_tables = ht_create(100, hash_function_key,
		compare_function_ints);
	int place_holder = 0;

	while(curr) {
		int server_id = ((server_memory *)curr->data)->server_id;

		// verific daca hashtable-ul server-ului a fost deja free-uit
		// la o iteratie anterioara:
		if (!ht_get(free_tables, &server_id)) {
			free_server_memory((server_memory *)curr->data);
			ht_put(free_tables, &server_id, sizeof(int),
				&place_holder, sizeof(int));
		}
		curr = curr->next;
	}
	ll_free(&(main_server->server_list));
	free(main_server);
	ht_free(free_tables);
}
