/*
 * vm.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>
#include "prims.h"
#include "utils/seg_vector.h"
#include "value.h"
#include "vm.h"

/* PRIVATE */

#define vm_stack_new() vector_new()
#define vm_stack_free(s) vector_free(s)

static vm_env_t *vm_env_new(vm_env_t *outer)
{
    return seg_vector_new(outer);
}

static value_t *vm_env_ref(vm_t *vm, int i, int j)
{
    return (value_t *)seg_vector_ref(vm->env, i, j);
}

static void vm_env_intern(vm_t *vm, value_t *o)
{
    seg_vector_push(vm->env, o);
}

static void vm_env_internbif(vm_t *vm, void *bif, int arity)
{
    value_t *v = value_bif_new(bif, arity);
    vm_env_intern(vm, v);
}

static value_t *vm_pop(vm_t *vm)
{
    return (value_t *)vector_pop(vm->stack);
}

static void vm_push(vm_t *vm, value_t *obj)
{
    vector_push(vm->stack, (intptr_t)obj);
}

static value_t *vm_top(vm_t *vm)
{
    return (value_t *)vector_top(vm->stack);
}

static void vm_execute_bif(vm_t *vm, value_t *f)
{
    value_t *res = NULL;
    switch (VALUE_BIF_ARITY(f)) {
        case 1: {
            value_t *arg1 = vm_pop(vm);
            res = ((bif_1)VALUE_BIF_PTR(f))(arg1);
            break;
        }
        case 2: {
            value_t *arg2 = vm_pop(vm);
            value_t *arg1 = vm_pop(vm);
            res = ((bif_2)VALUE_BIF_PTR(f))(arg1, arg2);
            break;
        }
        default :
            fprintf(stderr, "Unsupported arity of bif: %d\n", VALUE_BIF_ARITY(f));
    }
    vm_push(vm, res);
}

static void vm_execute_function(vm_t *vm)
{
    value_t *f = vm_pop(vm);
    if (VALUE_FUNC_ISBIF(f))
        vm_execute_bif(vm, f);
    else {
        fprintf(stderr, "Unsupported type of function\n");
    }
}

/* PUBLIC */

vm_t *vm_new(void)
{
    vm_t *vm = malloc(sizeof(*vm));
    vm->env = vm_env_new(NULL);
    vm->stack = vm_stack_new();
    vm_env_internbif(vm, bif_add, 2);
    vm_env_internbif(vm, bif_succ, 1);
    vm_env_internbif(vm, bif_div, 2);
    vm_env_internbif(vm, bif_equal, 2);
    vm_env_internbif(vm, bif_pred, 1);
    return vm;
}

void vm_free(vm_t *vm)
{
    seg_vector_free(vm->env);
    vm_stack_free(vm->stack);
    free(vm);
}

void vm_execute(vm_t *vm, ins_t *ins)
{
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        switch (bc->kind) {
            case BC_CALL:
                vm_execute_function(vm);
                break;
            case BC_GET:
                vm_push(vm, vm_env_ref(vm, BC_GET_I(bc), BC_GET_J(bc)));
                break;
            case BC_POP:
                vm_pop(vm);
                break;
            case BC_PUSH:
                vm_push(vm, BC_PUSH_OBJ(bc));
                break;
            default :
                fprintf(stderr, "Not support: %s\n", bc_name(bc));
                return;
        }
        i++;
    }
}

void vm_print_top(vm_t *vm, FILE *output)
{
    value_t *o = vm_top(vm);
    value_print(o, output);
    fputc('\n', output);
}
