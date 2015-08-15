/*
 * ast.h
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */

#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __ast_t ast_t;

typedef enum {
    AST_INVALID,
    AST_CONS,
    AST_END_OF_CONS,
    AST_END_OF_FILE,
    AST_IDENTIFIER,
    AST_INTEGER,
    AST_RIGHT_PAREN,
} ast_kind_t;

struct __ast_t {
    ast_kind_t kind;
    int line, column;
    union {
        string_t *ident_val;
        struct {
            ast_t *car;
            ast_t *cdr;
        } cons_val;
        int integer_val;
    } u;
};

extern ast_t *ast_invalid_new(void);
extern ast_t *ast_cons_new(ast_t *, ast_t *);
extern ast_t *ast_eoc_new(void);
extern ast_t *ast_eof_new(void);
extern ast_t *ast_ident_new(const char *);
extern ast_t *ast_int_new(int);
extern void ast_free(ast_t *);
extern void ast_print(ast_t *, FILE *);
extern int ast_cons_length(ast_t *);
extern int is_valof_form(ast_t *);

#define AST_CONS_CAR(c) ((c)->u.cons_val.car)
#define AST_CONS_CADR(c) AST_CONS_CAR( AST_CONS_CDR(c) )
#define AST_CONS_CADDR(c) AST_CONS_CAR( AST_CONS_CDDR(c) )
#define AST_CONS_CDR(c) ((c)->u.cons_val.cdr)
#define AST_CONS_CDDR(c) AST_CONS_CDR( AST_CONS_CDR(c) )
#define AST_IDENT_NAME(i) ((i)->u.ident_val->text)
#define AST_INT_VALUE(i) ((i)->u.integer_val)

#ifdef __cplusplus
}
#endif

#endif /* AST_H_ */
