/*
 * stack.h
 *
 *  Created on: 2015年8月4日
 *      Author: liutos
 */

#ifndef UTILS_STACK_H_
#define UTILS_STACK_H_

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef vector_t stack_t;

// Constructor
#define stack_new() vector_new()
// Destructor
#define stack_free(s) vector_free(s)
// Getter
#define stack_getpos(s) vector_curpos(s)
// Setter
#define stack_setpos(s, n) vector_setpos(s, n)
// Tester
#define stack_isempty(s) (stack_getpos(s) == 0)

#define stack_nth(s, n) vector_iref(s, n)
#define stack_pop(s) vector_pop(s)
#define stack_push(s, o) vector_push(s, (intptr_t)o)
#define stack_shrink(s, n) vector_shrink(s, n)
#define stack_top(s) vector_top(s)

#ifdef __cplusplus
}
#endif

#endif /* UTILS_STACK_H_ */
