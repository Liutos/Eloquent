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
#include "utils/hash_table.h"
#include "utils/string.h"
#include "utils/vector.h"
#include "value.h"

#define ERR 0
#define OK 1

typedef int (*compiler_rt_t)(compiler_t *, ast_t *, ins_t *);

/* PRIVATE */

static compiler_env_t *compiler_env_new(compiler_env_t *outer)
{
    compiler_env_t *env = malloc(sizeof(compiler_env_t));
    env->outer = outer;
    env->vars = vector_new();
    return env;
}

static void compiler_env_intern(compiler_env_t *env, const char *var, int *i, int *j)
{
    *i = 0;
    *j = env->vars->count;
    vector_push(env->vars, (intptr_t)var);
}

static int compiler_env_lookup(compiler_env_t *env, const char *var, int *i, int *j)
{
    int oi = 0;
    compiler_env_t *it = env;
    while (it != NULL) {
        int ii = vector_posif(it->vars, (intptr_t)var, (ele_comp_t)comp_str);
        if (ii != -1) {
            *i = oi;
            *j = ii;
            return 1;
        }
        it = it->outer;
        oi++;
    }
    return 0;
}

static compiler_rt_t compiler_getrt(compiler_t *comp, const char *name)
{
    return hash_table_get(comp->rts, (void *)name, NULL);
}

static void compiler_setrt(compiler_t *comp, const char *name, compiler_rt_t rt)
{
    hash_table_set(comp->rts, (void *)name, rt);
}

static int compiler_do_int(compiler_t *comp, ast_t *n, ins_t *ins)
{
    assert(ins != NULL);
    value_t *v = value_int_new(AST_INT_VALUE(n));
    ins_push(ins, bc_push_new(v));
    return 1;
}

static int compiler_do_ident(compiler_t *comp, ast_t *id, ins_t *ins)
{
    int i = 0, j = 0;
    if (compiler_env_lookup(comp->env, AST_IDENT_NAME(id), &i, &j) == 0) {
        string_printf(comp->error, "Line %d, column %d: Can't find value of `%s'", id->line, id->column, AST_IDENT_NAME(id));
        return ERR;
    }
    ins_push(ins, bc_get_new(i, j));
    return OK;
}

static int compiler_do_cons(compiler_t *comp, ast_t *cons, ins_t *ins)
{
    ast_t *op = AST_CONS_CAR(cons);
    if (op->kind != AST_IDENTIFIER) {
        string_printf(comp->error, "Line %d, column %d: The first element must be an identifier: %d", op->line, op->column, op->kind);
        return 0;
    }

    char *name = AST_IDENT_NAME(op);
    compiler_rt_t rt = compiler_getrt(comp, name);
    if (rt == NULL) {
        string_printf(comp->error, "Line %d, column %d: Don't know how to compile: %s", op->line, op->column, name);
        return 0;
    }
    return (*rt)(comp, AST_CONS_CDR(cons), ins);
}

static int compiler_do_begin(compiler_t *comp, ast_t *body, ins_t *ins)
{
    while (body->kind == AST_CONS && AST_CONS_CDR(body)->kind != AST_END_OF_CONS) {
        ast_t *expr = AST_CONS_CAR(body);
        compiler_do(comp, expr, ins);
        ins_push(ins, bc_pop_new());
        body = AST_CONS_CDR(body);
    }
    return compiler_do(comp, AST_CONS_CAR(body), ins);
}

static int compiler_do_set(compiler_t *comp, ast_t *body, ins_t *ins)
{
    ast_t *var = AST_CONS_CAR(body);
    ast_t *expr = AST_CONS_CAR( AST_CONS_CDR(body) );
    compiler_do(comp, expr, ins);
    int i = 0, j = 0;
    if (compiler_env_lookup(comp->env, AST_IDENT_NAME(var), &i, &j) == ERR) {
        compiler_env_intern(comp->env, AST_IDENT_NAME(var), &i, &j);
    }
    ins_push(ins, bc_set_new(i, j));
    return 1;
}

/* PUBLIC */

compiler_t *compiler_new(void)
{
    compiler_t *c = malloc(sizeof(compiler_t));
    c->error = string_new();
    c->env = compiler_env_new(NULL);
    c->rts = hash_table_new(hash_str, comp_str);
    compiler_setrt(c, "begin", compiler_do_begin);
    compiler_setrt(c, "set", compiler_do_set);
    return c;
}

void compiler_free(compiler_t *c)
{
    string_free(c->error);
    hash_table_free(c->rts);
    free(c);
}

int compiler_do(compiler_t *comp, ast_t *ast, ins_t *ins)
{
    assert(ins != NULL);
    switch (ast->kind) {
        case AST_IDENTIFIER:
            return compiler_do_ident(comp, ast, ins);
        case AST_INTEGER:
            return compiler_do_int(comp, ast, ins);
        case AST_CONS:
            return compiler_do_cons(comp, ast, ins);
        default :
            string_printf(comp->error, "Don't know how to compile: %d", ast->kind);
            return ERR;
    }
}
