/*
 * hash_table.h
 *
 *  Created on: 2013年9月28日
 *      Author: liutos
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

typedef unsigned int (*hash_fn_t)(void *);
typedef int (*comp_fn_t)(void *, void *);
typedef struct ht_slot_t ht_slot_t;
typedef struct hash_table_t hash_table_t;

/* General Hash Table Definition */
struct ht_slot_t {
  void *key;
  void *value;
  ht_slot_t *next;
};

// slots: An array for storing key-values
// length: Length of slots
// hash_fn: Pointer to function for generating hash value used as index in slots
// comp_fn: Pointer to function for comparing two keys when their hash value is equal
struct hash_table_t {
  ht_slot_t **slots;
  int length;
  hash_fn_t hash_fn;
  comp_fn_t comp_fn;
};

extern hash_table_t *make_hash_table(int, hash_fn_t, comp_fn_t);
extern void *search_ht(void *, hash_table_t *);
extern void set_ht(void *, void *, hash_table_t *);

/* Hash Table */
#define sl_key(x) ((x)->key)
#define sl_value(x) ((x)->value)
#define sl_next(x) ((x)->next)
#define ht_slots(x) ((x)->slots)
#define ht_length(x) ((x)->length)
#define ht_hash_fn(x) ((x)->hash_fn)
#define ht_comp_fn(x) ((x)->comp_fn)

#endif /* HASH_TABLE_H_ */
