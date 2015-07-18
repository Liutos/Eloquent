#include <stdio.h>
#include <stdlib.h>
#include "env.h"
#include "interp.h"
#include "value.h"

/* PRIVATE */

static value_t *bif_add(value_t *n1, value_t *n2)
{
    return value_int_new(VALUE_INT_VALUE(n1) + VALUE_INT_VALUE(n2));
}

static value_t *interp_get(interp_t *interp, char *name)
{
    return env_get(interp->env, name, NULL);
}

static void interp_set(interp_t *interp, char *name, value_t *value)
{
    env_set(interp->env, name, value);
}

static void interp_setbif(interp_t *interp, char *name, void *bif_ptr)
{
    value_t *val = value_bif_new(bif_ptr);
    interp_set(interp, name, val);
}

static void interp_initbif(interp_t *interp)
{
    interp_setbif(interp, "+", bif_add);
}

static value_kind_t interp_execute_bif(interp_t *interp, value_t *bif, ast_t *args, value_t **value)
{
    ast_t *arg_form1 = AST_CONS_CAR(args);
    value_t *arg1 = NULL;
    if (interp_execute(interp, arg_form1, &arg1) == VALUE_INVALID) {
        if (value != NULL)
            *value = arg1;
        return VALUE_INVALID;
    }

    ast_t *arg_form2 = AST_CONS_CAR( AST_CONS_CDR(args) );
    value_t *arg2 = NULL;
    if (interp_execute(interp, arg_form2, &arg2) == VALUE_INVALID) {
        if (value != NULL)
            *value = arg2;
        return VALUE_INVALID;
    }

    value_t *res = ((bif_2)VALUE_BIF_PTR(bif))(arg1, arg2);
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
