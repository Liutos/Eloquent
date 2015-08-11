#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "bytecode.h"
#include "env.h"
#include "utils/string.h"
#include "value.h"

/* PRIVATE */

static value_t *value_alloc(value_kind_t type)
{
    value_t *v = malloc(sizeof(*v));
    elo_type(v) = type;
    return v;
}

static void value_function_print(value_t *v, FILE *output)
{
    if (VALUE_FUNC_ISBIF(v)) {
        fprintf(output, "#<%p>", VALUE_BIF_PTR(v));
    } else if (!VALUE_FUNC_ISCMP(v)) {
        fprintf(output, "#<");
        ast_print(VALUE_UDF_PARS(v), output);
        fputc(' ', output);
        ast_print(VALUE_UDF_BODY(v), output);
        fprintf(output, ">");
    } else {
        fprintf(output, "#<%p>", v);
    }
}

static void value_print_cons(value_t *cons, FILE *output)
{
    value_t *car = VALUE_CONS_CAR(cons);
    value_t *cdr = VALUE_CONS_CDR(cons);
    fputc('(', output);
    value_print(car, output);
    if (elo_type(cdr) != VALUE_END_OF_CONS) {
        fprintf(output, " . ");
        value_print(cdr, output);
    }
    fputc(')', output);
}

/* PUBLIC */

const char *value_names[] = {
        VALUE_KIND(STRINGIFY)
};

value_t *value_bif_new(void *bif_ptr, unsigned int arity)
{
    value_t *v = value_alloc(VALUE_FUNCTION);
    VALUE_FUNC_ARITY(v) = arity;
    VALUE_FUNC_ISBIF(v) = 1;
    VALUE_FUNC_ISCMP(v) = 0;
    VALUE_BIF_PTR(v) = bif_ptr;
    return v;
}

value_t *value_cons_new(value_t *car, value_t *cdr)
{
    value_t *v = value_alloc(VALUE_CONS);
    VALUE_CONS_CAR(v) = car;
    VALUE_CONS_CDR(v) = cdr;
    return v;
}

value_t *value_eoc_new(void)
{
    return value_alloc(VALUE_END_OF_CONS);
}

value_t *value_error_new(const char *msg)
{
    value_t *v = value_alloc(VALUE_ERROR);
    v->u.err_val.msg = string_new();
    string_assign(v->u.err_val.msg, msg);
    return v;
}

value_t *value_error_newf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char msg[256] = {0};
    vsnprintf(msg, sizeof(msg), fmt, ap);
    return value_error_new(msg);
}

value_t *value_float_new(double num)
{
    value_t *v = value_alloc(VALUE_FLOAT);
    VALUE_FLOAT_VALUE(v) = num;
    return v;
}

value_t *value_int_new(int num)
{
    value_t *v = value_alloc(VALUE_INT);
    v->u.int_val = num;
    return v;
}

value_t *value_type_new(const char *name)
{
    value_t *v = value_alloc(VALUE_TYPE);
    VALUE_TYPE_NAME(v) = ident_intern(name);
    return v;
}

value_t *value_ucf_new(int arity, ins_t *code)
{
    value_t *v = value_alloc(VALUE_FUNCTION);
    VALUE_FUNC_ARITY(v) = arity;
    VALUE_FUNC_ISBIF(v) = 0;
    VALUE_FUNC_ISCMP(v) = 1;
    VALUE_UCF_CODE(v) = code;
    return v;
}

value_t *value_udf_new(int arity, ast_t *pars, ast_t *body, env_t *env)
{
    value_t *v = value_alloc(VALUE_FUNCTION);
    VALUE_FUNC_ARITY(v) = arity;
    VALUE_FUNC_ISBIF(v) = 0;
    VALUE_FUNC_ISCMP(v) = 0;
    VALUE_FUNC_ENV(v) = env;
    VALUE_UDF_PARS(v) = pars;
    VALUE_UDF_BODY(v) = body;
    return v;
}

void value_print(value_t *v, FILE *output)
{
    switch (v->kind) {
        case VALUE_CONS:
            value_print_cons(v, output);
            break;
        case VALUE_END_OF_CONS:
            fprintf(output, "()");
            break;
        case VALUE_ERROR:
            fprintf(output, "ERROR: %s", VALUE_ERR_MSG(v));
            break;
        case VALUE_FLOAT:
            fprintf(output, "%f", VALUE_FLOAT_VALUE(v));
            break;
        case VALUE_FUNCTION:
            value_function_print(v, output);
            break;
        case VALUE_INT:
            fprintf(output, "%d", v->u.int_val);
            break;
        case VALUE_TYPE:
            fprintf(output, "#<TYPE %s>", IDENT_NAME( VALUE_TYPE_NAME(v) ));
            break;
        default :
            fprintf(stderr, "Don't know how to print value: %d", v->kind);
            exit(1);
    }
}

int value_isequal(value_t *v1, value_t *v2)
{
    switch (v1->kind) {
        case VALUE_INT:
            return VALUE_INT_VALUE(v1) == VALUE_INT_VALUE(v2);
        default :
            return v1 == v2;
    }
}
