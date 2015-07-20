/*
 * bytecode.h
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __bytecode_t bytecode_t;

#define BC_KIND(op) \
    op(BC_POP), \
    op(BC_PUSH),
#define BC_IDENTIFY(bc) bc

typedef enum {
    BC_KIND(BC_IDENTIFY)
} bytecode_kind_t;

struct __bytecode_t {
    bytecode_kind_t kind;
    union {
        void *push_ptr;
    } u;
};

extern bytecode_t *bc_pop_new(void);
extern bytecode_t *bc_push_new(void *);
extern void bytecode_free(bytecode_t *);

#ifdef __cplusplus
}
#endif

#endif /* BYTECODE_H_ */
