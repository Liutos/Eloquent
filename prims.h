/*
 * prims.h
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */

#ifndef PRIMS_H_
#define PRIMS_H_

#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __prim_t prim_t;

struct __prim_t {
    const char *name;
    void *func_ptr;
    int arity;
};

extern prim_t prims[];
extern size_t prims_num;

extern value_t *bif_add(value_t *, value_t *);
extern value_t *bif_sub(value_t *, value_t *);
extern value_t *bif_succ(value_t *);
extern value_t *bif_pred(value_t *);
extern value_t *bif_div(value_t *, value_t *);
extern value_t *bif_equal(value_t *, value_t *);
extern value_t *bif_i2d(value_t *);
extern value_t *bif_ge(value_t *, value_t *);
extern value_t *bif_mul(value_t *, value_t *);

#ifdef __cplusplus
}
#endif

#endif /* PRIMS_H_ */
