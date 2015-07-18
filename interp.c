#include <stdio.h>
#include <stdlib.h>
#include "env.h"
#include "interp.h"
#include "value.h"

/* PRIVATE */

static value_t *interp_get(interp_t *interp, char *name)
{
    return env_get(interp->env, name, NULL);
}

static value_kind_t interp_execute_cons(interp_t *interp, ast_t *ast, value_t **value)
{
    ast_t *op = ast->u.cons_val.car;
    char *name = op->u.ident_val->text;
    value_t *val = interp_get(interp, name);
    if (val != NULL) {
        if (value != NULL)
            *value = val;
        return val->kind;
    } else {
        fprintf(stderr, "Can't find value of `%s'\n", name);
        exit(1);
    }
}

static value_kind_t interp_execute_ident(interp_t *interp, ast_t *ast, value_t **value)
{
    if (value != NULL)
        *value = value_int_new(233);
    return VALUE_INT;
}

/* PUBLIC */

interp_t *interp_new(void)
{
    interp_t *i = malloc(sizeof(interp_t));
    i->env = env_new(env_empty_new());
    return i;
}

void interp_free(interp_t *i)
{
    env_free(i->env);
    free(i);
}

value_kind_t interp_execute(interp_t *interp, ast_t *ast, value_t **value)
{
    switch (ast->kind) {
        case AST_CONS:
            return interp_execute_cons(interp, ast, value);
        case AST_IDENTIFIER:
            return interp_execute_ident(interp, ast, value);
        default :
            fprintf(stderr, "Don't know how to execute: %d", ast->kind);
            exit(1);
    }
}
