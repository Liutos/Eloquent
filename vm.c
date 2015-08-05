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
#include "env.h"
#include "prims.h"
#include "utils/seg_vector.h"
#include "utils/stack.h"
#include "value.h"
#include "vm.h"

/* PRIVATE */

static value_env_t *vm_env_new(value_env_t *outer)
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

static void vm_env_internbcf(vm_t *vm, void *bcf, int arity)
{
    ins_t *code = ins_new();
    ((bcf_t)bcf)(code);
    value_t *f = value_ucf_new(arity, code);
    vm_env_intern(vm, f);
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
    switch (VALUE_FUNC_ARITY(f)) {
        case 1: {
            value_t *arg1 = vm_pop(vm);
            res = elo_apply1(f, vm->denv, arg1);
        }
        break;
        case 2: {
            value_t *arg2 = vm_pop(vm);
            value_t *arg1 = vm_pop(vm);
            res = elo_apply2(f, vm->denv, arg1, arg2);
        }
        break;
        default :
            fprintf(stderr, "Unsupported arity of bif: %d\n", VALUE_FUNC_ARITY(f));
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
    vm->stack = stack_new();
    vm->sys_stack = stack_new();
    vm->denv = env_new(env_empty_new());

    int i = 0;
    while (i < prims_num) {
        prim_t *p = &prims[i];
        if (!p->is_compiled)
            vm_env_internbif(vm, p->func_ptr, p->arity);
        else
            vm_env_internbcf(vm, p->func_ptr, p->arity);
        i++;
    }
    return vm;
}

void vm_free(vm_t *vm)
{
    seg_vector_free(vm->env);
    stack_free(vm->stack);
    free(vm);
}

#define RESTORE \
    /* 恢复指令指针 */ \
    i = (int)stack_pop(vm->sys_stack); \
    /* 恢复指令信息 */ \
    ins = (ins_t *)stack_pop(vm->sys_stack); \
    /* 恢复环境信息 */ \
    vm->env = (value_env_t *)stack_pop(vm->sys_stack); \
    vm->denv = vm->denv->outer

void vm_execute(vm_t *vm, ins_t *ins)
{
    int i = 0;
    while (i < ins_length(ins)) {
        bytecode_t *bc = ins_ref(ins, i);
        switch (bc->opcode) {
            case BC_ARGS: {
                int i = BC_ARGS_ARITY(bc) - 1;
                for (; i >= 0; i--) {
                    value_t *obj = vm_iref(vm, i);
                    vm_env_intern(vm, obj);
                }
                stack_shrink(vm->stack, BC_ARGS_ARITY(bc));
            }
            break;
            case BC_CALL: {
                value_t *f = vm_pop(vm);
                if (BC_CALL_NARGS(bc) != VALUE_FUNC_ARITY(f)) {
                    vm_push(vm, value_error_newf("Incorrect number of arguments. Expecting %d but get %d ones", VALUE_FUNC_ARITY(f), BC_CALL_NARGS(bc)));
                    break;
                }

                if (VALUE_FUNC_ISBIF(f))
                    vm_execute_bif(vm, f);
                else {
                    /* 保存环境信息 */
                    stack_push(vm->sys_stack, vm->env);
                    /* 保存指令信息 */
                    stack_push(vm->sys_stack, ins);
                    /* 保存指令指针 */
                    stack_push(vm->sys_stack, i);
                    /* 初始化环境、字节码序列和指令指针 */
                    vm->env = vm_env_new(VALUE_UCF_ENV(f));
                    vm->denv = env_new(vm->denv);
                    ins = VALUE_UCF_CODE(f);
                    i = -1;
                    /* 开始执行新的字节码指令 */
                }
            }
            break;
__check_exception:
            case BC_CHKEX: {
                value_t *top = vm_top(vm);
                if (elo_ERRORP(top)) {
                    if (!stack_isempty(vm->sys_stack)) {
                        RESTORE;
                        /* 将运行时错误压回参数栈 */
                        vm_push(vm, top);
                    } else
                        return;
                }
            }
            break;
            case BC_DGET: {
                int is_found;
                value_t *object = env_get(vm->denv, BC_DGET_NAME(bc), &is_found);
                if (!is_found) {
                    vm_push(vm, value_error_newf("Undefined dynamic variable: %s", BC_DGET_NAME(bc)));
                    goto __check_exception;
                }

                vm_push(vm, object);
            }
            break;
            case BC_DSET:
                env_set(vm->denv, BC_DSET_NAME(bc), vm_top(vm));
                break;
            case BC_FJUMP: {
                value_t *o = vm_pop(vm);
                if (o->kind == VALUE_INT && VALUE_INT_VALUE(o) == 0)
                    i = BC_FJUMP_INDEX(bc) - 1;
            }
            break;
            case BC_FUNC: {
                value_t *f = vm_top(vm);
                VALUE_UCF_ENV(f) = vm->env;
            }
            break;
            case BC_GET: {
                value_t *object = vm_env_ref(vm, BC_GET_I(bc), BC_GET_J(bc));
                if (object == NULL) {
                    vm_push(vm, value_error_newf("Undefined variable: %s", BC_GET_NAME(bc)));
                    goto __check_exception;
                }

                vm_push(vm, object);
            }
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
            }
            break;
            case BC_PUSH:
                vm_push(vm, BC_PUSH_OBJ(bc));
                break;
            case BC_RETURN:
                RESTORE;
                /* 开始执行旧的字节码指令 */
                break;
            case BC_SET:
                vm_env_set(vm, BC_SET_I(bc), BC_SET_J(bc));
                break;
            default :
                fprintf(stderr, "Not support: %s\n", bc_name(bc));
                exit(0);
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
