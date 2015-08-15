/*
 * bytecode.h
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <stdio.h>

#include "ident.h"
#include "utils/string.h"
#include "utils/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __value_t;

typedef struct __bytecode_t bytecode_t;
typedef vector_t ins_t;

#define BC_KIND(op) \
    op(BC_ADDR), \
    op(BC_ARGS), \
    op(BC_ASET), \
    op(BC_CALL), \
    op(BC_CHKEX), \
    op(BC_DGET), \
    op(BC_DSET), \
    op(BC_FJUMP), \
    op(BC_FUNC), \
    op(BC_GREF), \
    op(BC_GSET), \
    op(BC_JUMP), \
    op(BC_LABEL), \
    op(BC_NOPE), \
    op(BC_POP), \
    op(BC_PRINT), \
    op(BC_PUSH), \
    op(BC_REF), \
    op(BC_RETURN), \
    op(BC_SET), \
    op(BC_VALOF),

#define BC_IDENTIFY(bc) bc
#define BC_STRINGIFY(bc) #bc

typedef enum {
    BC_KIND(BC_IDENTIFY)
} opcode_t;

struct __bytecode_t {
    opcode_t opcode;
    union {
        struct { struct __value_t *ptr; } bc_push;
        struct { ident_t *var; } bc_addr;
        struct { int arity; } bc_args;
        struct { int nargs; } bc_call;
        struct { char *name; } bc_dget;
        struct { char *name; } bc_dset;
        struct {
            bytecode_t *label;
            int index;
        } bc_fjump;
        struct {
            bytecode_t *label;
            int index;
        } bc_jump;
        struct { ident_t *name; } bc_label;
        struct {
            int i, j;
            char *name;
        } bc_gref, bc_ref;
        struct {
            int i, j;
            char *name;
        } bc_gset, bc_set;
    } u;
};

extern bytecode_t *bc_addr_new(const char *);
extern bytecode_t *bc_args_new(int);
extern bytecode_t *bc_aset_new(void);
extern bytecode_t *bc_call_new(int);
extern bytecode_t *bc_chkex_new(void);
extern bytecode_t *bc_dget_new(char *);
extern bytecode_t *bc_dset_new(char *);
extern bytecode_t *bc_fjump_new(bytecode_t *);
extern bytecode_t *bc_func_new(void);
extern bytecode_t *bc_gref_new(int, int, char *);
extern bytecode_t *bc_gset_new(int, int, char *);
extern bytecode_t *bc_jump_new(bytecode_t *);
extern bytecode_t *bc_label_new(const char *);
extern bytecode_t *bc_nope_new(void);
extern bytecode_t *bc_pop_new(void);
extern bytecode_t *bc_print_new(void);
extern bytecode_t *bc_push_new(void *);
extern bytecode_t *bc_ref_new(int, int, char *);
extern bytecode_t *bc_set_new(int, int, char *);
extern bytecode_t *bc_return_new(void);
extern bytecode_t *bc_valof_new(void);
extern void bc_print(bytecode_t *, FILE *);
extern const char *bc_name(bytecode_t *);

extern void ins_pretty_print(ins_t *, FILE *, int);

#define ins_new() vector_new()
#define ins_length(ins) ((ins)->count)
#define ins_push(ins, v) vector_push(ins, (intptr_t)v)
#define ins_ref(ins, i) (bytecode_t *)vector_ref(ins, i);

#define BC_OPCODE(b) ((b)->opcode)
#define BC_ADDR_VAR(a) ((a)->u.bc_addr.var)
#define BC_ADDR_NAME(a) IDENT_NAME( BC_ADDR_VAR(a) )
#define BC_ARGS_ARITY(a) ((a)->u.bc_args.arity)
#define BC_CALL_NARGS(b) ((b)->u.bc_call.nargs)
#define BC_DGET_NAME(b) ((b)->u.bc_dget.name)
#define BC_DSET_NAME(b) ((b)->u.bc_dset.name)
#define BC_FJUMP_INDEX(f) ((f)->u.bc_fjump.index)
#define BC_FJUMP_LABEL(f) ((f)->u.bc_fjump.label)
#define BC_FJUMP_LABEL_NAME(f) BC_LABEL_NAME( BC_FJUMP_LABEL(f) )
#define BC_JUMP_INDEX(f) ((f)->u.bc_jump.index)
#define BC_JUMP_LABEL(j) ((j)->u.bc_jump.label)
#define BC_JUMP_LABEL_NAME(j) BC_LABEL_NAME(BC_JUMP_LABEL(bc))
#define BC_LABEL_IDENT(l) ((l)->u.bc_label.name)
#define BC_LABEL_NAME(l) IDENT_NAME( BC_LABEL_IDENT(l) )
#define BC_PUSH_PTR(p) ((p)->u.bc_push.ptr)
#define BC_SET_I(s) ((s)->u.bc_set.i)
#define BC_SET_J(s) ((s)->u.bc_set.j)
#define BC_SET_NAME(s) ((s)->u.bc_set.name)
#define BC_REF_I(g) ((g)->u.bc_ref.i)
#define BC_REF_J(g) ((g)->u.bc_ref.j)
#define BC_REF_NAME(b) ((b)->u.bc_ref.name)

#ifdef __cplusplus
}
#endif

#endif /* BYTECODE_H_ */
