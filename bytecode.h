/*
 * bytecode.h
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <stdio.h>
#include "utils/string.h"
#include "utils/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __bytecode_t bytecode_t;
typedef vector_t ins_t;

#define BC_KIND(op) \
    op(BC_ARGS), \
    op(BC_CALL), \
    op(BC_FJUMP), \
    op(BC_FUNC), \
    op(BC_GET), \
    op(BC_JUMP), \
    op(BC_LABEL), \
    op(BC_NOPE), \
    op(BC_POP), \
    op(BC_PUSH), \
    op(BC_RETURN), \
    op(BC_SET),

#define BC_IDENTIFY(bc) bc
#define BC_STRINGIFY(bc) #bc

typedef enum {
    BC_KIND(BC_IDENTIFY)
} bytecode_kind_t;

struct __bytecode_t {
    bytecode_kind_t kind;
    union {
        void *push_ptr;
        struct {
            int i, j;
        } bc_get;
        struct {
            int i, j;
        } bc_set;
        struct {
            bytecode_t *label;
        } bc_fjump;
        struct {
            string_t *name;
        } bc_label;
        struct {
            bytecode_t *label;
        } bc_jump;
        struct {
            int arity;
        } bc_args;
    } u;
};

extern bytecode_t *bc_pop_new(void);
extern bytecode_t *bc_push_new(void *);
extern bytecode_t *bc_get_new(int, int);
extern bytecode_t *bc_set_new(int, int);
extern bytecode_t *bc_fjump_new(bytecode_t *);
extern bytecode_t *bc_jump_new(bytecode_t *);
extern bytecode_t *bc_label_new(const char *);
extern bytecode_t *bc_nope_new(void);
extern bytecode_t *bc_call_new(void);
extern bytecode_t *bc_args_new(int);
extern bytecode_t *bc_func_new(void);
extern bytecode_t *bc_return_new(void);
extern void bytecode_free(bytecode_t *);
extern const char *bc_name(bytecode_t *);

#define ins_length(ins) ((ins)->count)
#define ins_new() vector_new()
#define ins_push(ins, v) vector_push(ins, (intptr_t)v)
#define ins_ref(ins, i) (bytecode_t *)vector_ref(ins, i);
extern void ins_print(ins_t *, FILE *);
extern void ins_pretty_print(ins_t *, FILE *);

#define BC_ARGS_ARITY(a) ((a)->u.bc_args.arity)
#define BC_FJUMP_LABEL(f) ((f)->u.bc_fjump.label)
#define BC_FJUMP_LABEL_NAME(f) BC_LABEL_NAME( BC_FJUMP_LABEL(f) )
#define BC_GET_I(g) ((g)->u.bc_get.i)
#define BC_GET_J(g) ((g)->u.bc_get.j)
#define BC_JUMP_LABEL(j) ((j)->u.bc_jump.label)
#define BC_JUMP_LABEL_NAME(j) BC_LABEL_NAME((j)->u.bc_jump.label)
#define BC_LABEL_NAME(l) ((l)->u.bc_label.name->text)
#define BC_PUSH_OBJ(p) ((value_t *)(p)->u.push_ptr)

#ifdef __cplusplus
}
#endif

#endif /* BYTECODE_H_ */
