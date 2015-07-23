/*
 * vm.h
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */

#ifndef VM_H_
#define VM_H_

#include "bytecode.h"
#include "utils/seg_vector.h"
#include "utils/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __vm_t vm_t;
typedef vector_t vm_stack_t;
typedef seg_vector_t vm_env_t;

struct __vm_t {
    vm_env_t *env;
    vm_stack_t *stack;
};

extern vm_t *vm_new(void);
extern void vm_free(vm_t *);
extern void vm_execute(vm_t *, ins_t *);
extern void vm_print_top(vm_t *, FILE *);

#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
