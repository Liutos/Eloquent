/*
 * vm.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "prims.h"
#include "utils/seg_vector.h"
#include "value.h"
#include "vm.h"

#define __vm_stack_new() vector_new()
#define __vm_stack_push(vm, o) vector_push(vm->sys_stack, (intptr_t)o)
#define __vm_stack_pop(vm) vector_pop(vm->sys_stack)

/* PRIVATE */

/* LOG BEGIN */

static void vm_log_bc(vm_t *vm, bytecode_t *bc, const char *fmt, ...)
{
    char val[1024] = {0};
    bc_sprint(bc, val, sizeof(val));
    va_list ap;
    va_start(ap, fmt);
    char msg[256] = {0};
    vsnprintf(msg, sizeof(msg), fmt, ap);
    fprintf(vm->log, "[DEBUG] %s`bytecode=%s\n", msg, val);
}

static void vm_log_value(vm_t *vm, value_t *v, const char *fmt, ...)
{
    char val[1024] = {0};
    value_sprint(v, val, sizeof(val));
    va_list ap;
    va_start(ap, fmt);
    char msg[256] = {0};
    vsnprintf(msg, sizeof(msg), fmt, ap);
    fprintf(vm->log, "[DEBUG] %s`val=%s\n", msg, val);
}

static void vm_log_stack(vm_t *vm)
{
    int i = vm->stack->count - 1;
    while (i >= 0) {
        value_t *o = (value_t *)vector_ref(vm->stack, i);
        vm_log_value(vm, o, "msg=In stack`i=%d", i);
        i--;
    }
}

#define VMLOG_BC(vm, bc, fmt, ...) vm_log_bc(vm, bc, "pos=%s:%d`"fmt, __FILE__, __LINE__, __VA_ARGS__)
#define VMLOG_VAL(vm, v, fmt, ...) vm_log_value(vm, v, "pos=%s:%d`"fmt, __FILE__, __LINE__, __VA_ARGS__)

/* LOG END */

#define vm_stack_new() vector_new()
#define vm_stack_free(s) vector_free(s)
#define vm_stack_shrink(s, n) vector_shrink(s, n)
#define vm_stack_restore(vm) vector_setpos(vm->stack, vm->sp)
#define vm_stack_save(vm) vector_curpos(vm->stack)

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

static value_t *vm_top(vm_t *vm)
{
    return (value_t *)vector_top(vm->stack);
}

static void vm_env_set(vm_t *vm, int i, int j)
{
    value_t *val = vm_top(vm);
    if (i == -1 && j == -1)
        vm_env_intern(vm, val);
    else
        seg_vector_set(vm->env, val, i, j);
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

static value_t *vm_iref(vm_t *vm, int i)
{
    return (value_t *)vector_iref(vm->stack, i);
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

/* PRINT BEGIN */

static void vm_value_function_print(vm_t *vm, value_t *object)
{
    ins_t *ins = VALUE_UCF_CODE(object);
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        fprintf(stdout, "[%d]\t", i);
        bc_print(bc, stdout);
        fputc('\n', stdout);
        i++;
    }
}

static void vm_value_print(vm_t *vm, value_t *object)
{
    if (object->kind == VALUE_FUNCTION && VALUE_FUNC_ISCMP(object))
        vm_value_function_print(vm, object);
    else {
        value_print(object, stdout);
        fputc('\n', stdout);
    }
}

/* PRINT END */

/* PUBLIC */

vm_t *vm_new(void)
{
    vm_t *vm = malloc(sizeof(*vm));
    vm->env = vm_env_new(NULL);
    vm->stack = vm_stack_new();
    vm->sp = 0;
    vm->log = fopen("/tmp/eloquent.log", "w+");
    vm->sys_stack = __vm_stack_new();

    int i = 0;
    while (i < prims_num) {
        prim_t *p = &prims[i];
        vm_env_internbif(vm, p->func_ptr, p->arity);
        i++;
    }
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
        VMLOG_BC(vm, bc, "msg=Executing`i=%d", i);
        switch (bc->kind) {
            case BC_ARGS: {
                int i = BC_ARGS_ARITY(bc) - 1;
                for (; i >= 0; i--) {
                    value_t *obj = vm_iref(vm, i);
                    VMLOG_VAL(vm, obj, "msg=Moving`i=%d", i);
                    vm_env_intern(vm, obj);
                }
                vm_stack_shrink(vm->stack, BC_ARGS_ARITY(bc));
                break;
            }
            case BC_CALL: {
                value_t *f = vm_pop(vm);
                if (VALUE_FUNC_ISBIF(f))
                    vm_execute_bif(vm, f);
                else {
                    /* 保存环境信息 */
                    __vm_stack_push(vm, vm->env);
                    /* 保存栈指针信息 */
                    __vm_stack_push(vm, vm->sp);
                    /* 保存指令信息 */
                    __vm_stack_push(vm, ins);
                    /* 保存指令指针 */
                    __vm_stack_push(vm, i);
                    /* 初始化环境、字节码序列和指令指针 */
                    vm->env = vm_env_new(VALUE_UCF_ENV(f));
                    ins = VALUE_UCF_CODE(f);
                    i = -1;
                    /* 开始执行新的字节码指令 */
                }
                break;
            }
            case BC_CHKEX: {
                value_t *top = vm_top(vm);
                if (top->kind == VALUE_ERROR) {
                    vm_stack_restore(vm);
                    vm_push(vm, top);
                }
                break;
            }
            case BC_FJUMP: {
                value_t *o = vm_pop(vm);
                if (o->kind == VALUE_INT && VALUE_INT_VALUE(o) == 0)
                    i = BC_FJUMP_INDEX(bc) - 1;
                break;
            }
            case BC_FUNC: {
                value_t *f = vm_top(vm);
                VALUE_UCF_ENV(f) = vm->env;
                break;
            }
            case BC_GET:
                vm_push(vm, vm_env_ref(vm, BC_GET_I(bc), BC_GET_J(bc)));
                break;
            case BC_JUMP:
                i = BC_JUMP_INDEX(bc) - 1;
                break;
            case BC_NOPE:
                break;
            case BC_POP:
                vm_pop(vm);
                break;
            case BC_PRINT: {
                value_t *object = vm_top(vm);
                vm_value_print(vm, object);
                break;
            }
            case BC_PUSH:
                vm_push(vm, BC_PUSH_OBJ(bc));
                break;
            case BC_RETURN:
                /* 恢复指令指针 */
                i = (int)__vm_stack_pop(vm);
                /* 恢复指令信息 */
                ins = (ins_t *)__vm_stack_pop(vm);
                /* 恢复栈指针信息 */
                vm->sp = (size_t)__vm_stack_pop(vm);
                /* 恢复环境信息 */
                vm->env = (vm_env_t *)__vm_stack_pop(vm);
                /* 开始执行旧的字节码指令 */
                break;
            case BC_SET:
                vm_env_set(vm, BC_SET_I(bc), BC_SET_J(bc));
                break;
            default :
                fprintf(stderr, "Not support: %s\n", bc_name(bc));
                return;
        }
        i++;
        if (vm->stack->count != 0)
            vm_log_stack(vm);
    }
}

void vm_print_top(vm_t *vm, FILE *output)
{
    value_t *o = vm_top(vm);
    value_print(o, output);
    fputc('\n', output);
}

void vm_print_all(vm_t *vm, FILE *output)
{
    int i = vm->stack->count - 1;
    while (i >= 0) {
        value_t *o = (value_t *)vector_ref(vm->stack, i);
        fprintf(output, "%d\t", i);
        value_print(o, output);
        fputc('\n', output);
        i--;
    }
    vector_setpos(vm->stack, 0);
}

void vm_inject_print(ins_t *ins)
{
    ins_push(ins, bc_print_new());
}
