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
#include "utils/stack.h"
#include "value.h"
#include "vm.h"

/* PRIVATE */

static void vm_env_set(env_t *env, int i, int j, const char *name, value_t *val)
{
    if (i == -1 && j == -1)
        env_set(env, name, val);
    else
        env_update(env, i, j, val);
}

static void vm_execute_bif(vm_t *vm, value_t *f)
{
    value_t *res = NULL;
    switch (VALUE_FUNC_ARITY(f)) {
        case 0:
            res = elo_apply0(f, vm->denv);
            break;
        case 1: {
            value_t *arg1 = (value_t *)stack_pop(vm->stack);
            res = elo_apply1(f, vm->denv, arg1);
        }
        break;
        case 2: {
            value_t *arg2 = (value_t *)stack_pop(vm->stack);
            value_t *arg1 = (value_t *)stack_pop(vm->stack);
            res = elo_apply2(f, vm->denv, arg1, arg2);
        }
        break;
        default :
            fprintf(stderr, "Unsupported arity of bif: %d\n", VALUE_FUNC_ARITY(f));
            exit(0);
    }
    stack_push(vm->stack, res);
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

static void vm_goto(vm_t *vm, int index)
{
    vm->ip = index;
}

static void vm_restore(vm_t *vm, ins_t **ins)
{
    /* 恢复指令指针 */
    vm->ip = (int)stack_pop(vm->sys_stack);
    /* 恢复指令信息 */
    *ins = (ins_t *)stack_pop(vm->sys_stack);
    /* 恢复环境信息 */
    vm->env = (env_t *)stack_pop(vm->sys_stack);
    vm->denv = (env_t *)stack_pop(vm->sys_stack);
}

static void vm_save(vm_t *vm, ins_t *ins)
{
    /* 保存环境信息 */
    stack_push(vm->sys_stack, vm->denv);
    stack_push(vm->sys_stack, vm->env);
    /* 保存指令信息 */
    stack_push(vm->sys_stack, ins);
    /* 保存指令指针 */
    stack_push(vm->sys_stack, vm->ip);
}

static int vm_hasnext(vm_t *vm, ins_t *ins)
{
    return vm->ip < ins_length(ins);
}

static bytecode_t *vm_nextins(vm_t *vm, ins_t *ins)
{
    bytecode_t *bc = ins_ref(ins, vm->ip);
    vm->ip++;
    return bc;
}

/* PUBLIC */

vm_t *vm_new(void)
{
    vm_t *vm = malloc(sizeof(*vm));
    vm->ip = 0;
    /* Environments */
    vm->denv = env_new(env_empty_new());
    vm->env = env_new(env_empty_new());
    vm->init_env = elo_extend_env(env_new(env_empty_new()));
    vm->global_env = vm->init_env;

    vm->stack = stack_new();
    vm->sys_stack = stack_new();
    return vm;
}

void vm_free(vm_t *vm)
{
    env_free(vm->denv);
    env_free(vm->env);
    stack_free(vm->stack);
    stack_free(vm->sys_stack);
    free(vm);
}

void vm_execute(vm_t *vm, ins_t *ins)
{
    value_t *_fun_, *_val_;
    value_t **_addr_; // Used only by BC_ADDR
    vm->ip = 0;
    while (vm_hasnext(vm, ins)) {
        bytecode_t *bc = vm_nextins(vm, ins);
        switch (BC_OPCODE(bc)) {
            case BC_ADDR:
                _addr_ = env_getaddr(vm->env, BC_ADDR_NAME(bc));
                if (_addr_ == NULL)
                    _addr_ = env_getaddr(vm->global_env, BC_ADDR_NAME(bc));
                if (_addr_ == NULL) {
                    stack_push(vm->stack, value_error_newf("Undefined variable: %s", BC_ADDR_NAME(bc)));
                    goto __check_exception;
                }
                stack_push(vm->stack, value_ref_new(_addr_));
                break;
            case BC_ARGS:
                for (int i = BC_ARGS_ARITY(bc) - 1; i >= 0; i--) {
                    value_t *obj = (value_t *)stack_nth(vm->stack, i);
                    env_set(vm->env, NULL, obj);
                }
                stack_shrink(vm->stack, BC_ARGS_ARITY(bc));
                break;
            case BC_CALL:
                _fun_ = (value_t *)stack_pop(vm->stack);
                if (BC_CALL_NARGS(bc) != VALUE_FUNC_ARITY(_fun_)) {
                    stack_push(vm->stack, value_error_newf("Incorrect number of arguments. Expecting %d but get %d ones", VALUE_FUNC_ARITY(_fun_), BC_CALL_NARGS(bc)));
                    break;
                }

                if (VALUE_FUNC_ISBIF(_fun_))
                    vm_execute_bif(vm, _fun_);
                else {
                    vm_save(vm, ins);
                    /* 初始化环境、字节码序列和指令指针 */
                    vm->env = env_new(VALUE_FUNC_ENV(_fun_));
                    vm->denv = env_new(vm->denv);
                    ins = VALUE_UCF_CODE(_fun_);
                    vm_goto(vm, 0);
                    /* 开始执行新的字节码指令 */
                }
                break;
__check_exception:
            case BC_CHKEX:
                _val_ = (value_t *)stack_top(vm->stack);
                if (elo_ERRORP(_val_)) {
                    if (!stack_isempty(vm->sys_stack)) {
                        vm_restore(vm, &ins);
                        /* 将运行时错误压回参数栈 */
                        stack_push(vm->stack, _val_);
                    } else
                        return;
                }
                break;
            case BC_DGET:
                _val_ = env_get(vm->denv, BC_DGET_NAME(bc));
                if (_val_ == NULL) {
                    stack_push(vm->stack, value_error_newf("Undefined dynamic variable: %s", BC_DGET_NAME(bc)));
                    goto __check_exception;
                }

                stack_push(vm->stack, _val_);
                break;
            case BC_DSET:
                env_set(vm->denv, BC_DSET_NAME(bc), (value_t *)stack_top(vm->stack));
                break;
            case BC_FJUMP:
                _val_ = (value_t *)stack_pop(vm->stack);
                if (_val_->kind == VALUE_INT && VALUE_INT_VALUE(_val_) == 0)
                    vm_goto(vm, BC_FJUMP_INDEX(bc));
                break;
            case BC_GREF:
                _val_ = env_ref(vm->global_env, BC_REF_I(bc), BC_REF_J(bc));
                if (_val_ == NULL) {
                    stack_push(vm->stack, value_error_newf("Undefined variable: %s", BC_REF_NAME(bc)));
                    goto __check_exception;
                }

                stack_push(vm->stack, _val_);
                break;
            case BC_GSET:
                _val_ = (value_t *)stack_top(vm->stack);
                vm_env_set(vm->global_env, BC_SET_I(bc), BC_SET_J(bc), BC_SET_NAME(bc), _val_);
                break;
            case BC_FUNC:
                _val_ = (value_t *)stack_top(vm->stack);
                VALUE_FUNC_ENV(_val_) = vm->env;
                break;
            case BC_JUMP:
                vm_goto(vm, BC_JUMP_INDEX(bc));
                break;
            case BC_NOPE:
                break;
            case BC_POP:
                stack_pop(vm->stack);
                break;
            case BC_PRINT:
                _val_ = (value_t *)stack_top(vm->stack);
                vm_value_print(vm, _val_);
                break;
            case BC_PUSH:
                stack_push(vm->stack, BC_PUSH_PTR(bc));
                break;
            case BC_RETURN:
                vm_restore(vm, &ins);
                /* 开始执行旧的字节码指令 */
                break;
            case BC_REF:
                _val_ = env_ref(vm->env, BC_REF_I(bc), BC_REF_J(bc));
                if (_val_ == NULL) {
                    stack_push(vm->stack, value_error_newf("Undefined variable: %s", BC_REF_NAME(bc)));
                    goto __check_exception;
                }

                stack_push(vm->stack, _val_);
                break;
            case BC_SET:
                _val_ = (value_t *)stack_top(vm->stack);
                vm_env_set(vm->env, BC_SET_I(bc), BC_SET_J(bc), BC_SET_NAME(bc), _val_);
                break;
            case BC_VALOF:
                _val_ = (value_t *)stack_pop(vm->stack);
                if (elo_type(_val_) != VALUE_REF) {
                    stack_push(vm->stack, value_error_new("Argument of `valof' must be a reference"));
                    goto __check_exception;
                }
                stack_push(vm->stack, *VALUE_REF_ADDR(_val_));
                break;
            default :
                fprintf(stderr, "Not support: %s\n", bc_name(bc));
                exit(0);
        }
    }
}

void vm_inject_print(ins_t *ins)
{
    ins_push(ins, bc_print_new());
}
