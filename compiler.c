/*
 * compiler.c
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"
#include "compiler.h"
#include "utils/string.h"
#include "value.h"

#define ERR 0
#define OK 1

/* PRIVATE */

static int compiler_do_int(compiler_t *comp, ast_t *n, ins_t *ins)
{
    assert(ins != NULL);
    value_t *v = value_int_new(AST_INT_VALUE(n));
    ins_push(ins, bc_push_new(v));
    return 1;
}

/* PUBLIC */

compiler_t *compiler_new(void)
{
    compiler_t *c = malloc(sizeof(compiler_t));
    c->error = string_new();
    return c;
}

void compiler_free(compiler_t *c)
{
    string_free(c->error);
    free(c);
}

int compiler_do(compiler_t *comp, ast_t *ast, ins_t *ins)
{
    assert(ins != NULL);
    switch (ast->kind) {
        case AST_INTEGER:
            return compiler_do_int(comp, ast, ins);
        default :
            string_printf(comp->error, "Don't know how to compile: %d", ast->kind);
            return ERR;
    }
}
