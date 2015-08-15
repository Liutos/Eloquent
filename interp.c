#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "env.h"
#include "interp.h"
#include "misc.h"
#include "prims.h"
#include "utils/vector.h"
#include "value.h"

/* PRIVATE */

static syntax_t *syntax_new(void *ptr)
{
    syntax_t *s = malloc(sizeof(syntax_t));
    s->ptr = ptr;
    return s;
}

static syntax_t *interp_getbis(interp_t *interp, const char *name)
{
    return hash_table_get(interp->syntax_env, (void *)name, NULL);
}

static void interp_set(interp_t *interp, const char *name, value_t *value)
{
    env_set(interp->env, name, value);
}

/* SYNTAX BEGIN */

static value_kind_t bis_valof(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *expr = AST_CONS_CAR(body);
    value_t *val = NULL;
    if (interp_execute(interp, expr, &val) == VALUE_ERROR) {
        if (result != NULL)
            *result = val;
        return elo_type(val);
    }
    if (elo_type(val) != VALUE_REF) {
        if (result != NULL)
            *result = value_error_newf("Line %d, column %d: Value must be a reference", expr->line, expr->column);
        return VALUE_ERROR;
    }
    if (result != NULL)
        *result = *VALUE_REF_ADDR(val);
    return (*VALUE_REF_ADDR(val))->kind;
}

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

    if (is_valof_form(var)) {
        value_t *ref = NULL;
        if (interp_execute(interp, AST_CONS_CADR(var), &ref) == VALUE_ERROR) {
            if (result != NULL)
                *result = ref;
            return elo_type(ref);
        }
        *VALUE_REF_ADDR(ref) = val;
    } else
        interp_set(interp, AST_IDENT_NAME(var), val);
    if (result != NULL)
        *result = val;
    return val->kind;
}

/* Set dynamic variable */
static value_kind_t bis_dset(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *var = AST_CONS_CAR(body);
    ast_t *expr = AST_CONS_CAR( AST_CONS_CDR(body) );
    value_t *val = NULL;
    if (interp_execute(interp, expr, &val) == VALUE_ERROR) {
        if (result != NULL)
            *result = val;
        return val->kind;
    }

    env_set(interp->denv, AST_IDENT_NAME(var), val);
    if (result != NULL)
        *result = val;
    return val->kind;
}

/* Get value of dynamic variable */
static value_kind_t bis_dget(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *var = AST_CONS_CAR(body);
    char *name = AST_IDENT_NAME(var);
    value_t *val = env_get(interp->denv, name);
    if (val != NULL) {
        if (result != NULL)
            *result = val;
        return val->kind;
    } else {
        if (result != NULL)
            *result = value_error_newf("Line %d, column %d: Can't find value of `%s'", var->line, var->column, name);
        return VALUE_ERROR;
    }
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

static value_kind_t bis_begin(interp_t *interp, ast_t *body, value_t **result)
{
    while (body->kind == AST_CONS && AST_CONS_CDR(body)->kind != AST_END_OF_CONS) {
        ast_t *expr = AST_CONS_CAR(body);
        value_t *v = NULL;
        if (interp_execute(interp, expr, &v) == VALUE_ERROR) {
            if (result != NULL)
                *result = v;
            return v->kind;
        }
        body = AST_CONS_CDR(body);
    }
    return interp_execute(interp, AST_CONS_CAR(body), result);
}

static value_kind_t bis_define(interp_t *interp, ast_t *body, value_t **result)
{
    /* 构造符合set语法的表达式并再次求值 */
    ast_t *name = AST_CONS_CAR(body);
    ast_t *lpart = AST_CONS_CDR(body);
    ast_t *lexpr = ast_cons_new(ast_ident_new("lambda"), lpart);
    ast_t *sexpr = ast_cons_new(name, ast_cons_new(lexpr, ast_eoc_new()));
    return bis_set(interp, sexpr, result);
}

static value_kind_t bis_lambda(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *pars = AST_CONS_CAR(body);
    body = AST_CONS_CDR(body);
    value_t *val = value_udf_new(ast_cons_length(pars), pars, body, interp->env);
    if (result != NULL)
        *result = val;
    return val->kind;
}

static value_kind_t bis_refof(interp_t *interp, ast_t *body, value_t **result)
{
    ast_t *var = AST_CONS_CAR(body);
    if (var->kind != AST_IDENTIFIER) {
        if (result != NULL)
            *result = value_error_newf("Line %d, column %d: Operand of & must be an identifier", var->line, var->column);
        return VALUE_ERROR;
    }

    value_t **addr = env_getaddr(interp->env, AST_IDENT_NAME(var));
    if (result != NULL)
        *result = value_ref_new(addr);
    return VALUE_REF;
}

/* SYNTAX END */

static void interp_setbif(interp_t *interp, const char *name, void *bif_ptr, unsigned int arity)
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
    int i = 0;
    while (i < prims_num) {
        prim_t *p = &prims[i];
        if (!p->is_compiled)
            interp_setbif(interp, p->name, p->func_ptr, p->arity);
        i++;
    }

    interp_setbis(interp, "set", bis_set);
    interp_setbis(interp, "if", bis_if);
    interp_setbis(interp, "begin", bis_begin);
    interp_setbis(interp, "lambda", bis_lambda);
    interp_setbis(interp, "dset", bis_dset);
    interp_setbis(interp, "dget", bis_dget);
    interp_setbis(interp, "define", bis_define);
    interp_setbis(interp, "&", bis_refof);
    interp_setbis(interp, "valof", bis_valof);
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
    switch (VALUE_FUNC_ARITY(bif)) {
        case 0:
            res = elo_apply0(bif, interp->denv);
            break;
        case 1: {
            value_t *arg1 = (value_t *)vector_ref(vals, 0);
            res = elo_apply1(bif, interp->denv, arg1);
            break;
        }
        case 2: {
            value_t *arg1 = (value_t *)vector_ref(vals, 0);
            value_t *arg2 = (value_t *)vector_ref(vals, 1);
            res = elo_apply2(bif, interp->denv, arg1, arg2);
            break;
        }
        default :
            if (value != NULL)
                *value = value_error_newf("Don't support build in function of arity %d", VALUE_FUNC_ARITY(bif));
            return VALUE_ERROR;
    }

    if (value != NULL)
        *value = res;
    return res->kind;
}

static int interp_bind_args(interp_t *interp, ast_t *pars, ast_t *exprs, env_t *env, value_t **error)
{
    assert(error != NULL);
    while (pars->kind == AST_CONS) {
        ast_t *par = AST_CONS_CAR(pars);
        ast_t *expr = AST_CONS_CAR(exprs);
        value_t *val = NULL;
        if (interp_execute(interp, expr, &val) == VALUE_ERROR) {
            *error = val;
            return ERR;
        }
        env_set(env, AST_IDENT_NAME(par), val);
        pars = AST_CONS_CDR(pars);
        exprs = AST_CONS_CDR(exprs);
    }
    return OK;
}

static value_kind_t interp_execute_udf(interp_t *interp, value_t *f, ast_t *args, value_t **value)
{
    env_t *new_env = env_new(VALUE_FUNC_ENV(f));
    value_t *err = NULL;
    if (interp_bind_args(interp, VALUE_UDF_PARS(f), args, new_env, &err) == ERR) {
        if (value != NULL)
            *value = err;
        return err->kind;
    }

    env_t *old_env = interp->env;
    interp->env = new_env;
    interp->denv = env_new(interp->denv);
    value_kind_t kind = bis_begin(interp, VALUE_UDF_BODY(f), value);
    interp->env = old_env;
    interp->denv = interp->denv->outer;
    return kind;
}

static value_kind_t interp_execute_function(interp_t *interp, value_t *f, ast_t *args, value_t **value)
{
    int arity = VALUE_FUNC_ARITY(f);
    int nargs = ast_cons_length(args);
    if (nargs != arity) {
        if (value != NULL)
            *value = value_error_newf("Line %d, column %d: Incorrect number of arguments. Expecting %d but get %d", args->line, args->column, arity, nargs);
        return VALUE_ERROR;
    }
    if (VALUE_FUNC_ISBIF(f))
        return interp_execute_bif(interp, f, args, value);
    else
        return interp_execute_udf(interp, f, args, value);
}

static value_kind_t interp_execute_cons(interp_t *interp, ast_t *ast, value_t **value)
{
    ast_t *op = AST_CONS_CAR(ast);
    if (op->kind != AST_IDENTIFIER) {
        if (value != NULL)
            *value = value_error_newf("Line %d, column %d: The first element must be identifier", ast->line, ast->column);
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
            *value = value_error_newf("Line %d, column %d: The first element must evaluated to a function", ast->line, ast->column);
        return VALUE_ERROR;
    }

    return interp_execute_function(interp, op_value, AST_CONS_CDR(ast), value);
}

static value_kind_t interp_execute_ident(interp_t *interp, ast_t *ast, value_t **value)
{
    char *name = AST_IDENT_NAME(ast);
    value_t *val = env_get(interp->env, name);
    if (val != NULL) {
        if (value != NULL)
            *value = val;
        return val->kind;
    } else {
        if (value != NULL)
            *value = value_error_newf("Line %d, column %d: Can't find value of `%s'", ast->line, ast->column, name);
        return VALUE_ERROR;
    }
}

/* PUBLIC */

interp_t *interp_new(void)
{
    interp_t *i = malloc(sizeof(interp_t));
    i->env = env_new(env_empty_new());
    i->syntax_env = hash_table_new(hash_str, comp_str);
    i->denv = env_new(env_empty_new());
    interp_initbif(i);
    return i;
}

value_kind_t interp_execute(interp_t *interp, ast_t *ast, value_t **value)
{
    switch (ast->kind) {
        case AST_CONS:
            return interp_execute_cons(interp, ast, value);
        case AST_END_OF_CONS:
            if (value != NULL)
                *value = value_int_new(0);
            return VALUE_INT;
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
