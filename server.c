/* Copyright 2021 <Calugaritoiu Ion-Victor> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#include "load_balancer.h"
#include "hashtable.h"
#include "linkedlist.h"

/*
 * Functia initializeaza si aloca memorie pentru structura server:
 */
server_memory* init_server_memory() {
	server_memory *server = malloc (sizeof(server_memory));
	DIE(!server, "server malloc");

	server->ht = ht_create(NR_BUCKETS, hash_function_key, compare_function_ints);
	server->server_id = -1;

	return server;
}

/*
 * Functia stocheaza perechea cheie-valoare in server-ul respectiv:
 */
void server_store(server_memory* server, char* key, char* value) {
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

/*
 * Functia sterge perechea cheie-valoare din server-ul respectiv:
 */
void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server->ht, key);
}

/*
 * Functia returneaza valoarea corespunzatoare cheii din hashtable:
 */
char* server_retrieve(server_memory* server, char* key) {
	return (char *)ht_get(server->ht, key);
}

/*
 * Functia elibereaza memoria utilizata de campul "data" al unui server:
 */
void free_server_memory(server_memory* server) {
	ht_free(server->ht);
}
