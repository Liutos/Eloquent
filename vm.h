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
#include "utils/stack.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __vm_t vm_t;

struct __vm_t {
    env_t *denv;
    env_t *env;
    stack_t *stack;
    stack_t *sys_stack;
};

extern vm_t *vm_new(void);
extern void vm_free(vm_t *);
extern void vm_execute(vm_t *, ins_t *);
extern void vm_inject_print(ins_t *);
extern void vm_print_all(vm_t *, FILE *);
extern void vm_print_top(vm_t *, FILE *);

#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
