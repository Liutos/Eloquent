/*
 * ast.c
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#include <stdlib.h>
#include "ast.h"
#include "utils/string.h"

/* PUBLIC */

ast_t *ast_cons_new(ast_t *car, ast_t *cdr)
{
    ast_t *a = malloc(sizeof(ast_t));
    a->kind = AST_CONS;
    a->u.cons_val.car = car;
    a->u.cons_val.cdr = cdr;
    return a;
}

ast_t *ast_ident_new(const char *id)
{
    ast_t *a = malloc(sizeof(ast_t));
    a->kind = AST_IDENTIFIER;
    a->u.ident_val = string_new();
    string_assign(a->u.ident_val, id);
    return a;
}

void ast_free(ast_t *a)
{
    switch (a->kind) {
        case AST_CONS:
            ast_free(a->u.cons_val.car);
            ast_free(a->u.cons_val.cdr);
            break;
        case AST_IDENTIFIER:
            string_free(a->u.ident_val);
            break;
        default :;
    }
    free(a);
}
