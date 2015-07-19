#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "utils/string.h"
#include "value.h"

/* PRIVATE */

static void value_function_print(value_t *v, FILE *output)
{
    if (VALUE_FUNC_ISBIF(v)) {
        fprintf(output, "#<%p>", VALUE_BIF_PTR(v));
    } else {
        fprintf(output, "#<");
        ast_print(VALUE_UDF_PARS(v), output);
        fputc(' ', output);
        ast_print(VALUE_UDF_BODY(v), output);
        fprintf(output, ">");
    }
}

/* PUBLIC */

value_t *value_error_new(const char *msg)
{
    value_t *v = malloc(sizeof(value_t));
    v->kind = VALUE_ERROR;
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

value_t *value_int_new(int num)
{
    value_t *v = malloc(sizeof(value_t));
    v->kind = VALUE_INT;
    v->u.int_val = num;
    return v;
}

value_t *value_bif_new(void *bif_ptr, unsigned int arity)
{
    value_t *v = malloc(sizeof(value_t));
    v->kind = VALUE_FUNCTION;
    VALUE_FUNC_ISBIF(v) = 1;
    VALUE_BIF_ARITY(v) = arity;
    VALUE_BIF_PTR(v) = bif_ptr;
    return v;
}

value_t *value_udf_new(ast_t *pars, ast_t *body)
{
    value_t *v = malloc(sizeof(value_t));
    v->kind = VALUE_FUNCTION;
    VALUE_FUNC_ISBIF(v) = 0;
    VALUE_UDF_PARS(v) = pars;
    VALUE_UDF_BODY(v) = body;
    return v;
}

void value_free(value_t *v)
{
    switch (v->kind) {
        case VALUE_ERROR:
            string_free(v->u.err_val.msg);
            break;
        case VALUE_INT:
        default :;
    }
    free(v);
}

void value_print(value_t *v, FILE *output)
{
    switch (v->kind) {
        case VALUE_ERROR:
            fprintf(output, "ERROR: %s", VALUE_ERR_MSG(v));
            break;
        case VALUE_FUNCTION:
            value_function_print(v, output);
            break;
        case VALUE_INT:
            fprintf(output, "%d", v->u.int_val);
            break;
        default :
            fprintf(stderr, "Don't know how to print value: %d", v->kind);
            exit(1);
    }
}
