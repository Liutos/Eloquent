/*
 * bytecode.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"

/* PRIVATE */

static char *bc_names[] ={
        BC_KIND(BC_STRINGIFY)
};

bytecode_t *bc_new(bytecode_kind_t kind)
{
    bytecode_t *bc = malloc(sizeof(bytecode_t));
    bc->kind = kind;
    return bc;
}

void bc_print(bytecode_t *bc, FILE *output)
{
    switch (bc->kind) {
        case BC_POP:
        case BC_PUSH:
            fprintf(output, "%s", bc_names[bc->kind]);
            break;
        default :;
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

void bytecode_free(bytecode_t *bc)
{
    free(bc);
}

void ins_print(ins_t *ins, FILE *output)
{
    int i = 0;
    while (i < ins->count) {
        bytecode_t *bc = (bytecode_t *)vector_ref(ins, i);
        bc_print(bc, output);
        fputc('\n', output);
        i++;
    }
}
