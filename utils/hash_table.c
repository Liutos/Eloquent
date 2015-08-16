/*
 * hash_table.c
 *
 *  Created on: 2013年9月28日
 *      Author: liutos
 *
 * This file contains the definition of a general purpose hash table
 */
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/* PRIVATE */

static size_t hash_table_hash(hash_table_t *tbl, void *key)
{
    return (tbl->hash_func)(key) % tbl->capacity;
}

static int hash_table_iseq(hash_table_t *tbl, void *k1, void *k2)
{
    return (tbl->comp_func)(k1, k2);
}

/* PUBLIC */

hash_table_t *hash_table_new(hash_func_t hash_func, comp_func_t comp_func)
{
#define TABLE_SIZE 11
    hash_table_t *tbl = malloc(sizeof(hash_table_t));
    tbl->capacity = TABLE_SIZE;
    tbl->slots = calloc(tbl->capacity, sizeof(table_entry_t *));
    tbl->count = 0;
    tbl->comp_func = comp_func;
    tbl->hash_func = hash_func;
    return tbl;
#undef TABLE_SIZE
}

void hash_table_free(hash_table_t *tbl)
{
    free(tbl->slots);
    free(tbl);
}

void hash_table_set(hash_table_t *tbl, void *key, void *value)
{
    size_t i = hash_table_hash(tbl, key);
    table_entry_t *ent = tbl->slots[i];
    while (ent != NULL) {
        if (hash_table_iseq(tbl, ent->key, key))
            break;
        ent = ent->next;
    }
    if (ent == NULL) {
        ent = malloc(sizeof(table_entry_t));
        ent->key = key;
        ent->next = tbl->slots[i];
        tbl->slots[i] = ent;
    }
    ent->value = value;
}

void *hash_table_get(hash_table_t *tbl, void *key, int *is_found)
{
    size_t i = hash_table_hash(tbl, key);
    table_entry_t *ent = tbl->slots[i];
    while (ent != NULL) {
        if (hash_table_iseq(tbl, ent->key, key)) {
            if (is_found != NULL)
                *is_found = 1;
            return ent->value;
        }
        ent = ent->next;
    }
    if (is_found != NULL)
        *is_found = 0;
    return NULL;
}

size_t hash_ptr(void *ptr)
{
    return (size_t)ptr;
}

size_t hash_str(void *str)
{
    const char *_str = str;
    unsigned int seed = 131;
    unsigned int hash = 0;
    while (*_str != '\0') {
        hash = hash * seed + (*_str);
        _str++;
    }
    return hash & 0x7FFFFFFF;
}

int comp_ptr(void *_p1, void *_p2)
{
    return _p1 == _p2;
}

int comp_str(void *_s1, void *_s2)
{
    const char *s1 = _s1;
    const char *s2 = _s2;
    return strcmp(s1, s2) == 0;
}
