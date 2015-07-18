#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "env.h"
#include "interp.h"
#include "utils/vector.h"
#include "value.h"

#define ERR 0
#define OK 1

/* PRIVATE */

static value_t *bif_add(value_t *n1, value_t *n2)
{
    return value_int_new(VALUE_INT_VALUE(n1) + VALUE_INT_VALUE(n2));
}

static value_t *bif_succ(value_t *n)
{
    return value_int_new(VALUE_INT_VALUE(n) + 1);
}

static value_t *interp_get(interp_t *interp, char *name)
{
    return env_get(interp->env, name, NULL);
}

static void interp_set(interp_t *interp, char *name, value_t *value)
{
    env_set(interp->env, name, value);
}

static void interp_setbif(interp_t *interp, char *name, void *bif_ptr, unsigned int arity)
{
    value_t *val = value_bif_new(bif_ptr, arity);
    interp_set(interp, name, val);
}

static void interp_initbif(interp_t *interp)
{
    interp_setbif(interp, "+", bif_add, 2);
    interp_setbif(interp, "succ", bif_succ, 1);
}

static int interp_execute_args(interp_t *interp, ast_t *args, vector_t **_vals, value_t **error)
{
    assert(_vals != NULL);
    assert(error != NULL);

    vector_t *vals = vector_new();
    while (args->kind != AST_END_OF_CONS) {
        ast_t *ele = AST_CONS_CAR(args);
        value_t *val = NULL;
        if (interp_execute(interp, ele, &val) == VALUE_INVALID) {
            *error = val;
            return ERR;
        }
        vector_push(vals, (intptr_t)val);
        args = AST_CONS_CDR(args);
    }
    *_vals = vals;
    return OK;
}

static value_kind_t interp_execute_bif(interp_t *interp, value_t *bif, ast_t *args, value_t **value)
{
    value_t *error = NULL;
    vector_t *vals = NULL;
    if (interp_execute_args(interp, args, &vals, &error) == ERR) {
        if (value != NULL)
            *value = error;
        return VALUE_INVALID;
    }

    value_t *res = NULL;
    switch (VALUE_BIF_ARITY(bif)) {
        case 1: {
            value_t *arg1 = (value_t *)vector_ref(vals, 0);
            res = ((bif_1)VALUE_BIF_PTR(bif))(arg1);
            break;
        }
        case 2: {
            value_t *arg1 = (value_t *)vector_ref(vals, 0);
            value_t *arg2 = (value_t *)vector_ref(vals, 1);
            res = ((bif_2)VALUE_BIF_PTR(bif))(arg1, arg2);
            break;
        }
        default :
            if (value != NULL)
                *value = value_invalid_newf("Don't support build in function of arity %d", VALUE_BIF_ARITY(bif));
            return VALUE_INVALID;
    }

    if (value != NULL)
        *value = res;
    return res->kind;
}

static value_kind_t interp_execute_cons(interp_t *interp, ast_t *ast, value_t **value)
{
    ast_t *op = AST_CONS_CAR(ast);
    if (op->kind != AST_IDENTIFIER) {
        if (value != NULL)
            *value = value_invalid_new("The first element must be identifier");
        return VALUE_INVALID;
    }

    value_t *op_value = NULL;
    value_kind_t kind = interp_execute(interp, op, &op_value);
    if (kind == VALUE_INVALID) {
        if (value != NULL)
            *value = op_value;
        return VALUE_INVALID;
    }

    if (kind != VALUE_FUNCTION) {
        if (value != NULL)
            *value = value_invalid_new("The first element must evaluated to a function");
        return VALUE_INVALID;
    }

    return interp_execute_bif(interp, op_value, AST_CONS_CDR(ast), value);
}

static value_kind_t interp_execute_ident(interp_t *interp, ast_t *ast, value_t **value)
{
    char *name = AST_IDENT_NAME(ast);
    value_t *val = interp_get(interp, name);
    if (val != NULL) {
        if (value != NULL)
            *value = val;
        return val->kind;
    } else {
        if (value != NULL)
            *value = value_invalid_newf("Can't find value of `%s'", name);
        return VALUE_INVALID;
    }
}

/* PUBLIC */

interp_t *interp_new(void)
{
    interp_t *i = malloc(sizeof(interp_t));
    i->env = env_new(env_empty_new());
    interp_initbif(i);
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
        case AST_INTEGER:
            if (value != NULL)
                *value = value_int_new(AST_INT_VALUE(ast));
            return VALUE_INT;
        default :
            fprintf(stderr, "Don't know how to execute: %d", ast->kind);
            exit(1);
    }
}
