/*
 * bytecode.h
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <stdio.h>
#include "utils/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __bytecode_t bytecode_t;
typedef vector_t ins_t;

#define BC_KIND(op) \
    op(BC_POP), \
    op(BC_PUSH),
#define BC_IDENTIFY(bc) bc
#define BC_STRINGIFY(bc) #bc

typedef enum {
    BC_KIND(BC_IDENTIFY)
} bytecode_kind_t;

struct __bytecode_t {
    bytecode_kind_t kind;
    union {
        void *push_ptr;
    } u;
};

extern bytecode_t *bc_pop_new(void);
extern bytecode_t *bc_push_new(void *);
extern void bytecode_free(bytecode_t *);

#define ins_new() vector_new()
#define ins_push(ins, v) vector_push(ins, (intptr_t)v)
extern void ins_print(ins_t *, FILE *);

#ifdef __cplusplus
}
#endif

#endif /* BYTECODE_H_ */
