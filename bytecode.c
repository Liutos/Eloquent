/*
 * bytecode.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <stdlib.h>
#include "bytecode.h"

/* PRIVATE */

bytecode_t *bc_new(bytecode_kind_t kind)
{
    bytecode_t *bc = malloc(sizeof(bytecode_t));
    bc->kind = kind;
    return bc;
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
