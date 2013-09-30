/*
 * hash_table.h
 *
 *  Created on: 2013年9月28日
 *      Author: liutos
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include "type.h"

extern hash_table_t *make_hash_table(int, hash_fn_t, comp_fn_t);
extern void *search_ht(void *, hash_table_t *);
extern void set_ht(void *, void *, hash_table_t *);

#endif /* HASH_TABLE_H_ */
