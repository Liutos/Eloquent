/*
 * vm.h
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */

#ifndef VM_H_
#define VM_H_

#include <stdio.h>

#include "bytecode.h"
#include "env.h"
#include "utils/hash_table.h"
#include "utils/stack.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __vm_t vm_t;

struct __vm_t {
    int depth;
    int ip; // Instruction Pointer. Points to the next instruction to be executed
    env_t *denv; // Current dynamic environment
    env_t *env; // Current lexical environment
    env_t *global_env; // The unique global environment
    env_t *init_env; // The environment with primitives
    hash_table_t *traced_objs;
    stack_t *stack;
    stack_t *sys_stack;
    value_t *fun; // The function executed currently
};

extern vm_t *vm_new(void);
extern void vm_free(vm_t *);
extern void vm_execute(vm_t *, ins_t *);
extern void vm_inject_print(ins_t *);

#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
