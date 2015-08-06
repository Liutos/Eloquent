/*
 * prims.c
 *
 * Functions defined in this file should not be directly invoked in C code, but just
 * be used as implementation of primitives.
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include "env.h"
#include "bytecode.h"
#include "prims.h"
#include "value.h"

#define BIF_NAME(name) bif_##name
#define DENV denv
#define DEFINE_BIF(name, ...) value_t *BIF_NAME(name)(env_t *DENV, __VA_ARGS__)
#define DEFINE_BIF1(name, arg1) DEFINE_BIF(name, value_t *arg1)
#define DEFINE_BIF2(name, arg1, arg2) DEFINE_BIF(name, value_t *arg1, value_t *arg2)

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

DEFINE_BIF2(add, n1, n2)
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

DEFINE_BIF2(sub, n1, n2)
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

DEFINE_BIF2(mul, n1, n2)
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

DEFINE_BIF2(div, n1, n2)
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

DEFINE_BIF1(succ, n)
{
    elo_INT_ASSERT(n);
    return value_int_new(VALUE_INT_VALUE(n) + 1);
}

DEFINE_BIF1(pred, n)
{
    elo_INT_ASSERT(n);
    return value_int_new(VALUE_INT_VALUE(n) - 1);
}

DEFINE_BIF2(equal, v1, v2)
{
    return value_int_new(value_isequal(v1, v2));
}

DEFINE_BIF1(i2d, n)
{
    elo_INT_ASSERT(n);
    return value_float_new(VALUE_INT_VALUE(n));
}

DEFINE_BIF2(ge, n1, n2)
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
    ins_push(ins, bc_ref_new(0, 0, NULL));
    ins_push(ins, bc_print_new());
    ins_push(ins, bc_return_new());
}

#define _BIF(_name, _ptr, _arity) { .is_compiled = 0, .name = _name, .func_ptr = BIF_NAME(_ptr), .arity = _arity }
#define _BCF(_name, _ptr, _arity) { .is_compiled = 1, .name = _name, .func_ptr = _ptr, .arity = _arity }

prim_t prims[] = {
        _BIF("+", add, 2),
        _BIF("-", sub, 2),
        _BIF("*", mul, 2),
        _BIF("/", div, 2),
        _BIF("succ", succ, 1),
        _BIF("pred", pred, 1),
        _BIF("i2d", i2d, 1),
        _BIF("=", equal, 2),
        _BIF(">=", ge, 2),
        _BCF("print", bcf_print, 1),
};
size_t prims_num = sizeof(prims) / sizeof(prim_t);
