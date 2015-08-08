/*
 * prims.h
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */

#ifndef PRIMS_H_
#define PRIMS_H_

#include "bytecode.h"
#include "env.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __prim_t prim_t;
typedef void (*bcf_t)(ins_t *);

typedef value_t *(*bif_0)(env_t *);
typedef value_t *(*bif_1)(env_t *, value_t *);
typedef value_t *(*bif_2)(env_t *, value_t *, value_t *);

/* Primitives invocation */
#define elo_apply0(f, denv) (((bif_0)VALUE_BIF_PTR(f))(denv))
#define elo_apply1(f, denv, arg1) (((bif_1)VALUE_BIF_PTR(f))(denv, arg1))
#define elo_apply2(f, denv, arg1, arg2) (((bif_2)VALUE_BIF_PTR(f))(denv, arg1, arg2))

struct __prim_t {
    int is_compiled;
    const char *name;
    void *func_ptr;
    int arity;
};

extern prim_t prims[];
extern size_t prims_num;

extern env_t *elo_extend_env(env_t *);

#ifdef __cplusplus
}
#endif

#endif /* PRIMS_H_ */
