/*
 * prims.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include "prims.h"
#include "value.h"

/* PUBLIC */

value_t *bif_add(value_t *n1, value_t *n2)
{
    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) + VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_float_new(VALUE_INT_VALUE(n1) + VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_float_new(VALUE_FLOAT_VALUE(n1) + VALUE_INT_VALUE(n2));
    return value_float_new(VALUE_FLOAT_VALUE(n1) + VALUE_FLOAT_VALUE(n2));
}

value_t *bif_sub(value_t *n1, value_t *n2)
{
    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) - VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_float_new(VALUE_INT_VALUE(n1) - VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_float_new(VALUE_FLOAT_VALUE(n1) - VALUE_INT_VALUE(n2));
    return value_float_new(VALUE_FLOAT_VALUE(n1) - VALUE_FLOAT_VALUE(n2));
}

value_t *bif_mul(value_t *n1, value_t *n2)
{
    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) * VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_float_new(VALUE_INT_VALUE(n1) * VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_float_new(VALUE_FLOAT_VALUE(n1) * VALUE_INT_VALUE(n2));
    return value_float_new(VALUE_FLOAT_VALUE(n1) * VALUE_FLOAT_VALUE(n2));
}

value_t *bif_succ(value_t *n)
{
    return value_int_new(VALUE_INT_VALUE(n) + 1);
}

value_t *bif_pred(value_t *n)
{
    return value_int_new(VALUE_INT_VALUE(n) - 1);
}

value_t *bif_div(value_t *n1, value_t *n2)
{
    if (n2->kind == VALUE_INT && VALUE_INT_VALUE(n2) == 0)
        return value_error_new("Divided by zero");
    if (n2->kind == VALUE_FLOAT && VALUE_FLOAT_VALUE(n2) == 0)
        return value_error_new("Divided by zero");

    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) / VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_float_new(VALUE_INT_VALUE(n1) / VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_float_new(VALUE_FLOAT_VALUE(n1) / VALUE_INT_VALUE(n2));
    return value_float_new(VALUE_FLOAT_VALUE(n1) / VALUE_FLOAT_VALUE(n2));
}

value_t *bif_equal(value_t *v1, value_t *v2)
{
    return value_int_new(value_isequal(v1, v2));
}

value_t *bif_i2d(value_t *n)
{
    return value_float_new(VALUE_INT_VALUE(n));
}

value_t *bif_ge(value_t *n1, value_t *n2)
{
    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) >= VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_int_new(VALUE_INT_VALUE(n1) >= VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_FLOAT_VALUE(n1) >= VALUE_INT_VALUE(n2));
    return value_int_new(VALUE_FLOAT_VALUE(n1) >= VALUE_FLOAT_VALUE(n2));
}

prim_t prims[] = {
        { .name = "+", .func_ptr = bif_add, .arity = 2 },
        { .name = "-", .func_ptr = bif_sub, .arity = 2 },
        { .name = "*", .func_ptr = bif_mul, .arity = 2 },
        { .name = "/", .func_ptr = bif_div, .arity = 2 },
        { .name = "succ", .func_ptr = bif_succ, .arity = 1 },
        { .name = "pred", .func_ptr = bif_pred, .arity = 1 },
        { .name = "i2d", .func_ptr = bif_i2d, .arity = 1 },
        { .name = "=", .func_ptr = bif_equal, .arity = 2 },
        { .name = ">=", .func_ptr = bif_ge, .arity = 2 },
};
size_t prims_num = sizeof(prims) / sizeof(prim_t);
