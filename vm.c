/*
 * vm.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>
#include "utils/seg_vector.h"
#include "vm.h"

/* PRIVATE */

#define vm_stack_new() vector_new()
#define vm_stack_free(s) vector_free(s)

/* PUBLIC */

vm_t *vm_new(void)
{
    vm_t *vm = malloc(sizeof(*vm));
    vm->env = seg_vector_new(NULL);
    vm->stack = vm_stack_new();
    return vm;
}

void vm_free(vm_t *vm)
{
    seg_vector_free(vm->env);
    vm_stack_free(vm->stack);
    free(vm);
}
