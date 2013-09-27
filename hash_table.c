/*
 * hash_table.c
 *
 *  Created on: 2013年9月28日
 *      Author: liutos
 *
 * This file contains the definition of a general purpose hash table
 */
#include <stdio.h>

#include <gc/gc.h>

#include "hash_table.h"
#include "type.h"

ht_slot_t *make_slot(void *key, void *value, ht_slot_t *next) {
  ht_slot_t *sl = GC_MALLOC(sizeof(*sl));
  sl_key(sl) = key;
  sl_value(sl) = value;
  sl_next(sl) = next;
  return sl;
}

hash_table_t *make_hash_table(int length, hash_fn_t hash_fn, comp_fn_t comp_fn) {
  hash_table_t *ht = GC_MALLOC(sizeof(hash_table_t));
  ht_slots(ht) = GC_MALLOC(length * sizeof(ht_slot_t *));
  ht_length(ht) = length;
  ht_hash_fn(ht) = hash_fn;
  ht_comp_fn(ht) = comp_fn;
  return ht;
}

unsigned int compute_index(void *key, hash_table_t *ht) {
  hash_fn_t fn = ht_hash_fn(ht);
  int length = ht_length(ht);
  return (*fn)(key) % length;
}

int compare_in_ht(void *k1, void *k2, hash_table_t *ht) {
  comp_fn_t fn = ht_comp_fn(ht);
  return (*fn)(k1, k2);
}

ht_slot_t *raw_search_ht(void *key, hash_table_t *ht) {
  int index = compute_index(key, ht);
  ht_slot_t *sl = ht_slots(ht)[index];
  while (sl != NULL) {
    void *sk = sl_key(sl);
    if (compare_in_ht(sk, key, ht) == 0)
      return sl;
    sl = sl_next(sl);
  }
  return NULL;
}

void *search_ht(void *key, hash_table_t *ht) {
  ht_slot_t *sl = raw_search_ht(key, ht);
  if (sl != NULL)
    return sl_value(sl);
  else
    return NULL;
}

void set_ht(void *key, void *value, hash_table_t *ht) {
  ht_slot_t *sl = raw_search_ht(key, ht);
  if (sl != NULL)
    sl_value(sl) = value;
  else {
    unsigned int index = compute_index(key, ht);
    ht_slot_t *org = ht_slots(ht)[index];
    ht_slot_t *sl = make_slot(key, value, org);
    ht_slots(ht)[index] = sl;
  }
}
