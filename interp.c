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

static syntax_t *syntax_new(void *ptr)
{
    syntax_t *s = malloc(sizeof(syntax_t));
    s->ptr = ptr;
    return s;
}

static value_t *bif_add(value_t *n1, value_t *n2)
{
    return value_int_new(VALUE_INT_VALUE(n1) + VALUE_INT_VALUE(n2));
}

static value_t *bif_succ(value_t *n)
{
    return value_int_new(VALUE_INT_VALUE(n) + 1);
}

static value_t *bif_div(value_t *n1, value_t *n2)
{
    if (VALUE_INT_VALUE(n2) == 0)
        return value_error_new("Divided by zero");
    return value_int_new(VALUE_INT_VALUE(n1) / VALUE_INT_VALUE(n2));
}

static value_t *interp_get(interp_t *interp, char *name)
{
    return env_get(interp->env, name, NULL);
}

static syntax_t *interp_getbis(interp_t *interp, const char *name)
{
    return hash_table_get(interp->syntax_env, (void *)name, NULL);
}

static void interp_set(interp_t *interp, char *name, value_t *value)
{
    env_set(interp->env, name, value);
}

/* SYNTAX BEGIN */

static value_kind_t bis_set(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *var = AST_CONS_CAR(body);
    ast_t *expr = AST_CONS_CAR( AST_CONS_CDR(body) );
    value_t *val = NULL;
    if (interp_execute(interp, expr, &val) == VALUE_ERROR) {
        if (result != NULL)
            *result = val;
        return val->kind;
    }

    interp_set(interp, AST_IDENT_NAME(var), val);
    if (result != NULL)
        *result = val;
    return val->kind;
}

static value_kind_t bis_if(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *pred = AST_CONS_CAR(body);
    ast_t *p = AST_CONS_CAR( AST_CONS_CDR(body) );
    ast_t *e = AST_CONS_CAR( AST_CONS_CDR( AST_CONS_CDR(body) ) );
    value_t *pred_val = NULL;
    if (interp_execute(interp, pred, &pred_val) == VALUE_ERROR) {
        if (result != NULL)
            *result = pred_val;
        return pred_val->kind;
    }

    if (pred_val->kind == VALUE_INT && VALUE_INT_VALUE(pred_val) == 0) {
        return interp_execute(interp, e, result);
    } else {
        return interp_execute(interp, p, result);
    }
}

/* SYNTAX END */

static void interp_setbif(interp_t *interp, char *name, void *bif_ptr, unsigned int arity)
{
    value_t *val = value_bif_new(bif_ptr, arity);
    interp_set(interp, name, val);
}

static void interp_setbis(interp_t *interp, const char *name, void *bis_ptr)
{
    syntax_t *s = syntax_new(bis_ptr);
    hash_table_set(interp->syntax_env, (void *)name, s);
}

static void interp_initbif(interp_t *interp)
{
    interp_setbif(interp, "+", bif_add, 2);
    interp_setbif(interp, "succ", bif_succ, 1);
    interp_setbif(interp, "/", bif_div, 2);

    interp_setbis(interp, "set", bis_set);
    interp_setbis(interp, "if", bis_if);
}

static value_kind_t interp_execute_syntax(interp_t *interp, syntax_t *bis, ast_t *body, value_t **result)
{
    return ((bis_t)(bis->ptr))(interp, body, result);
}

static int interp_execute_args(interp_t *interp, ast_t *args, vector_t **_vals, value_t **error)
{
    assert(_vals != NULL);
    assert(error != NULL);

    vector_t *vals = vector_new();
    while (args->kind != AST_END_OF_CONS) {
        ast_t *ele = AST_CONS_CAR(args);
        value_t *val = NULL;
        if (interp_execute(interp, ele, &val) == VALUE_ERROR) {
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
        return error->kind;
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
                *value = value_error_newf("Don't support build in function of arity %d", VALUE_BIF_ARITY(bif));
            return VALUE_ERROR;
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
            *value = value_error_new("The first element must be identifier");
        return VALUE_ERROR;
    }

    char *name = AST_IDENT_NAME(op);
    syntax_t *bis = interp_getbis(interp, name);
    if (bis != NULL)
        return interp_execute_syntax(interp, bis, AST_CONS_CDR(ast), value);

    value_t *op_value = NULL;
    value_kind_t kind = interp_execute(interp, op, &op_value);
    if (kind == VALUE_ERROR) {
        if (value != NULL)
            *value = op_value;
        return kind;
    }

    if (kind != VALUE_FUNCTION) {
        if (value != NULL)
            *value = value_error_new("The first element must evaluated to a function");
        return VALUE_ERROR;
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
            *value = value_error_newf("Can't find value of `%s'", name);
        return VALUE_ERROR;
    }
}

/* PUBLIC */

interp_t *interp_new(void)
{
    interp_t *i = malloc(sizeof(interp_t));
    i->env = env_new(env_empty_new());
    i->syntax_env = hash_table_new(hash_str, comp_str);
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
