/* Copyright 2021 <Calugaritoiu Ion-Victor> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "hashtable.h"

#define MAX_BUCKET_SIZE 64

/*
 * Functii de comparare a cheilor:
 */
int
compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */
unsigned int
hash_function_int(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int
hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;  /* hash * 33 + c */

	return hash;
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 */
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
	int (*compare_function)(void*, void*))
{
	hashtable_t *ht = malloc(sizeof(hashtable_t));
	DIE(ht == NULL, "hashtable malloc");

	ht->compare_function = compare_function;
	ht->hash_function = hash_function;
	ht->hmax = hmax;
	ht->size = 0;

	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
	DIE(ht->buckets == NULL, "ht->buckets malloc");

	for (unsigned int i = 0; i < hmax; ++i)
		ht->buckets[i] = ll_create(sizeof(info));

	return ht;
}

/*
 * Functia adauga o noua pereche cheie-valoare in hashtable:
 */
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
	unsigned int value_size)
{
	int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
    info *infoo = malloc(sizeof(*infoo));
    DIE(infoo == NULL, "info malloc");

	infoo->key = malloc(key_size);
	DIE(infoo->key == NULL, "key malloc");

	memcpy(infoo->key, key, key_size);

	infoo->value = malloc(value_size);
	DIE(infoo->value == NULL, "value malloc");

	memcpy(infoo->value, value, value_size);

	while(curr) {
		if(ht->compare_function(key, ((info *)curr->data)->key) == 0) {
			memcpy(((info *)curr->data)->value, infoo->value, value_size);
			free(infoo->key);
			free(infoo->value);
			free(infoo);
			return;
		}
		curr = curr->next;
	}
	ht->size++;
	ll_add_nth_node(ht->buckets[index], 0, infoo);

	free(infoo);
}

/*
 * Functia returneaza valoarea asociata cheii primite ca parametru:
 */
void *
ht_get(hashtable_t *ht, void *key)
{
	int index = (ht->hash_function(key)) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;

	while(curr) {
		if(ht->compare_function(key, ((info *)curr->data)->key) == 0) {
			return ((info *)curr->data)->value;
		}
		curr = curr->next;
	}
	return NULL;
}

/*
 * Functie care returneaza:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable folosind functia put
 * 0, altfel.
 */
int
ht_has_key(hashtable_t *ht, void *key)
{
	if(ht_get(ht, key))
		return 1;

	return 0;
}

/*
 * Functia elimina din hashtable intrarea asociata cheii key.
 */
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	int index = (ht->hash_function(key)) % ht->hmax;
	int nr = 0;

	ll_node_t *curr = ht->buckets[index]->head;

	while(curr) {
		if(ht->compare_function(key, ((info *)curr->data)->key) == 0) {
			curr = ll_remove_nth_node(ht->buckets[index], nr);
			free(((info *)curr->data)->key);
			free(((info *)curr->data)->value);
			free(curr->data);
			free(curr);
			ht->size--;
			return;
		}
		nr++;
		curr = curr->next;
	}
}

/*
 * Functia elibereaza complet toata memoria utilizata de hashtable:
 */
void
ht_free(hashtable_t *ht)
{
	for(unsigned int i = 0; i < ht->hmax; ++i) {
		ll_node_t *curr = ht->buckets[i]->head;

		while(curr) {
			free(((info *)curr->data)->key);
			free(((info *)curr->data)->value);
			curr = curr->next;
		}
		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}
