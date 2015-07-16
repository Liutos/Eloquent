/*
 * ast.h
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */

#ifndef AST_H_
#define AST_H_

#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __ast_t ast_t;

typedef enum {
    AST_INVALID,
    AST_CONS,
    AST_IDENTIFIER,
} ast_kind_t;

struct __ast_t {
    ast_kind_t kind;
    union {
        string_t *ident_val;
        struct {
            ast_t *car;
            ast_t *cdr;
        } cons_val;
    } u;
};

extern ast_t *ast_cons_new(ast_t *, ast_t *);
extern ast_t *ast_ident_new(const char *);
extern void ast_free(ast_t *);

#ifdef __cplusplus
}
#endif

#endif /* AST_H_ */
