/*
 * prims.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include "bytecode.h"
#include "prims.h"
#include "value.h"

#define elo_INT_ASSERT(var) \
    do { \
        if (!elo_INTP(var)) \
            return value_error_newf("%s: Argument must be a integer", __func__); \
    } while (0)

#define elo_NUMBER_ASSERT(var) \
    do { \
        if (!elo_NUMBERP(var)) \
            return value_error_newf("%s: Argument must be a number", __func__); \
    } while (0)

/* PUBLIC */

value_t *bif_add(value_t *n1, value_t *n2)
{
    elo_NUMBER_ASSERT(n1);
    elo_NUMBER_ASSERT(n2);
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
    elo_NUMBER_ASSERT(n1);
    elo_NUMBER_ASSERT(n2);
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
    elo_NUMBER_ASSERT(n1);
    elo_NUMBER_ASSERT(n2);
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
    elo_INT_ASSERT(n);
    return value_int_new(VALUE_INT_VALUE(n) + 1);
}

value_t *bif_pred(value_t *n)
{
    elo_INT_ASSERT(n);
    return value_int_new(VALUE_INT_VALUE(n) - 1);
}

value_t *bif_div(value_t *n1, value_t *n2)
{
    elo_NUMBER_ASSERT(n1);
    elo_NUMBER_ASSERT(n2);
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
    elo_INT_ASSERT(n);
    return value_float_new(VALUE_INT_VALUE(n));
}

value_t *bif_ge(value_t *n1, value_t *n2)
{
    elo_NUMBER_ASSERT(n1);
    elo_NUMBER_ASSERT(n2);
    if (n1->kind == VALUE_INT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_INT_VALUE(n1) >= VALUE_INT_VALUE(n2));
    if (n1->kind == VALUE_INT && n2->kind == VALUE_FLOAT)
        return value_int_new(VALUE_INT_VALUE(n1) >= VALUE_FLOAT_VALUE(n2));
    if (n1->kind == VALUE_FLOAT && n2->kind == VALUE_INT)
        return value_int_new(VALUE_FLOAT_VALUE(n1) >= VALUE_INT_VALUE(n2));
    return value_int_new(VALUE_FLOAT_VALUE(n1) >= VALUE_FLOAT_VALUE(n2));
}

/* Built-in Compiled Functions */

void bcf_print(ins_t *ins)
{
    ins_push(ins, bc_args_new(1));
    ins_push(ins, bc_get_new(0, 0, NULL));
    ins_push(ins, bc_print_new());
    ins_push(ins, bc_return_new());
}

#define _BIF(_name, _ptr, _arity) { .is_compiled = 0, .name = _name, .func_ptr = _ptr, .arity = _arity }
#define _BCF(_name, _ptr, _arity) { .is_compiled = 1, .name = _name, .func_ptr = _ptr, .arity = _arity }

prim_t prims[] = {
        _BIF("+", bif_add, 2),
        _BIF("-", bif_sub, 2),
        _BIF("*", bif_mul, 2),
        _BIF("/", bif_div, 2),
        _BIF("succ", bif_succ, 1),
        _BIF("pred", bif_pred, 1),
        _BIF("i2d", bif_i2d, 1),
        _BIF("=", bif_equal, 2),
        _BIF(">=", bif_ge, 2),
        _BCF("print", bcf_print, 1),
};
size_t prims_num = sizeof(prims) / sizeof(prim_t);
