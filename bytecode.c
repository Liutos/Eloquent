/*
 * bytecode.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"
#include "value.h"

/* PRIVATE */

static const char *bc_names[] ={
        BC_KIND(BC_STRINGIFY)
};

static bytecode_t *bc_new(bytecode_kind_t kind)
{
    bytecode_t *bc = malloc(sizeof(bytecode_t));
    bc->kind = kind;
    return bc;
}

static void bc_print(bytecode_t *bc, FILE *output)
{
    switch (bc->kind) {
        case BC_ARGS:
            fprintf(output, "%s %d", bc_name(bc), BC_ARGS_ARITY(bc));
            break;
        case BC_FJUMP:
            fprintf(output, "%s %d", bc_name(bc), BC_FJUMP_INDEX(bc));
            break;
        case BC_JUMP:
            fprintf(output, "%s %d", bc_name(bc), BC_JUMP_INDEX(bc));
            break;
        case BC_LABEL:
            fprintf(output, "%s %s", bc_name(bc), BC_LABEL_NAME(bc));
            break;
        case BC_SET:
            fprintf(output, "%s %d %d", bc_name(bc), bc->u.bc_set.i, bc->u.bc_set.j);
            break;
        case BC_PUSH:
            fprintf(output, "%s ", bc_name(bc));
            value_print(BC_PUSH_OBJ(bc), output);
            break;
        case BC_GET:
            fprintf(output, "%s %d %d", bc_name(bc), bc->u.bc_get.i, bc->u.bc_get.j);
            break;
        case BC_POP:
        default :
            fprintf(output, "%s", bc_name(bc));
    }
}

/* PUBLIC */

bytecode_t *bc_pop_new(void)
{
    return bc_new(BC_POP);
}

bytecode_t *bc_push_new(void *ptr)
{
    bytecode_t *bc = bc_new(BC_PUSH);
    bc->u.push_ptr = ptr;
    return bc;
}

bytecode_t *bc_get_new(int i, int j)
{
    bytecode_t *bc = bc_new(BC_GET);
    bc->u.bc_get.i = i;
    bc->u.bc_get.j = j;
    return bc;
}

bytecode_t *bc_set_new(int i, int j)
{
    bytecode_t *bc = bc_new(BC_SET);
    bc->u.bc_set.i = i;
    bc->u.bc_set.j = j;
    return bc;
}

bytecode_t *bc_fjump_new(bytecode_t *label)
{
    bytecode_t *bc = bc_new(BC_FJUMP);
    BC_FJUMP_INDEX(bc) = -1;
    bc->u.bc_fjump.label = label;
    return bc;
}

bytecode_t *bc_jump_new(bytecode_t *label)
{
    bytecode_t *bc = bc_new(BC_JUMP);
    BC_JUMP_LABEL(bc) = label;
    BC_JUMP_INDEX(bc) = -1;
    return bc;
}

bytecode_t *bc_label_new(const char *name)
{
    bytecode_t *bc = bc_new(BC_LABEL);
    bc->u.bc_label.name = string_new();
    string_assign(bc->u.bc_label.name, name);
    return bc;
}

bytecode_t *bc_nope_new(void)
{
    return bc_new(BC_NOPE);
}

bytecode_t *bc_call_new(void)
{
    return bc_new(BC_CALL);
}

bytecode_t *bc_args_new(int arity)
{
    bytecode_t *bc = bc_new(BC_ARGS);
    bc->u.bc_args.arity = arity;
    return bc;
}

bytecode_t *bc_func_new(void)
{
    return bc_new(BC_FUNC);
}

bytecode_t *bc_return_new(void)
{
    return bc_new(BC_RETURN);
}

void bytecode_free(bytecode_t *bc)
{
    free(bc);
}

void ins_print(ins_t *ins, FILE *output)
{
    int i = 0;
    while (i < ins->count) {
        bytecode_t *bc = ins_ref(ins, i);
        bc_print(bc, output);
        fputc('\n', output);
        i++;
    }
}

void ins_pretty_print(ins_t *ins, FILE *output)
{
    int i = 0;
    while (i < ins->count) {
        bytecode_t *bc = ins_ref(ins, i);
        if (bc->kind == BC_LABEL)
            fprintf(output, "%s", BC_LABEL_NAME(bc));
        else {
            fprintf(output, "\t\t");
            bc_print(bc, output);
            fputc('\n', output);
        }
        i++;
    }
}

const char *bc_name(bytecode_t *bc)
{
    return bc_names[bc->kind];
}
