/*
 * hash_table.h
 *
 *  Created on: 2013年9月28日
 *      Author: liutos
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __hash_table_t hash_table_t;
typedef struct __table_entry_t table_entry_t;

typedef size_t (*hash_func_t)(void *);
typedef int (*comp_func_t)(void *, void *);

struct __table_entry_t {
    void *key;
    void *value;
    table_entry_t *next;
};

struct __hash_table_t {
    table_entry_t **slots;
    size_t capacity;
    size_t count;
    hash_func_t hash_func;
    comp_func_t comp_func;
};

extern hash_table_t *hash_table_new(hash_func_t, comp_func_t);
extern void hash_table_free(hash_table_t *);
extern void hash_table_set(hash_table_t *, void *, void *);
extern void *hash_table_get(hash_table_t *, void *, int *);

extern size_t hash_str(void *);
extern int comp_str(void *, void *);

#ifdef __cplusplus
}
#endif

#endif /* HASH_TABLE_H_ */
