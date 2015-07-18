#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils/string.h"
#include "value.h"

/* PUBLIC */

value_t *value_invalid_new(const char *msg)
{
    value_t *v = malloc(sizeof(value_t));
    v->kind = VALUE_INVALID;
    v->u.invalid_msg = string_new();
    string_assign(v->u.invalid_msg, msg);
    return v;
}

value_t *value_invalid_newf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char msg[256] = {0};
    vsnprintf(msg, sizeof(msg), fmt, ap);
    return value_invalid_new(msg);
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

void value_free(value_t *v)
{
    switch (v->kind) {
        case VALUE_INVALID:
            string_free(v->u.invalid_msg);
            break;
        case VALUE_INT:
        default :;
    }
    free(v);
}

void value_print(value_t *v, FILE *output)
{
    switch (v->kind) {
        case VALUE_FUNCTION:
            fprintf(output, "#<%p>", VALUE_BIF_PTR(v));
            break;
        case VALUE_INT:
            fprintf(output, "%d", v->u.int_val);
            break;
        default :
            fprintf(stderr, "Don't know how to print value: %d", v->kind);
            exit(1);
    }
}
