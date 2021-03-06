/*
 * ast.c
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "utils/string.h"

/* PRIVATE */

static ast_t *ast_alloc(ast_kind_t kind)
{
    ast_t *a = malloc(sizeof(ast_t));
    a->kind = kind;
    return a;
}

static void ast_print_cons(ast_t *cons, FILE *output)
{
    ast_t *car = cons->u.cons_val.car;
    ast_t *cdr = cons->u.cons_val.cdr;
    fputc('(', output);
    ast_print(car, output);
    if (cdr->kind != AST_END_OF_CONS) {
        fprintf(output, " . ");
        ast_print(cdr, output);
    }
    fputc(')', output);
}

/* PUBLIC */

ast_t *ast_invalid_new(void)
{
    return ast_alloc(AST_INVALID);
}

ast_t *ast_cons_new(ast_t *car, ast_t *cdr)
{
    ast_t *a = ast_alloc(AST_CONS);
    a->u.cons_val.car = car;
    a->u.cons_val.cdr = cdr;
    return a;
}

ast_t *ast_eoc_new(void)
{
    return ast_alloc(AST_END_OF_CONS);
}

ast_t *ast_eof_new(void)
{
    return ast_alloc(AST_END_OF_FILE);
}

ast_t *ast_ident_new(const char *id)
{
    ast_t *a = ast_alloc(AST_IDENTIFIER);
    a->u.ident_val = string_new();
    string_assign(a->u.ident_val, id);
    return a;
}

ast_t *ast_int_new(int num)
{
    ast_t *a = ast_alloc(AST_INTEGER);
    AST_INT_VALUE(a) = num;
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

void ast_print(ast_t *a, FILE *output)
{
    switch (a->kind) {
        case AST_CONS:
            ast_print_cons(a, output);
            break;
        case AST_END_OF_CONS:
            fprintf(output, "()");
            break;
        case AST_IDENTIFIER:
            fprintf(output, "%s", a->u.ident_val->text);
            break;
        case AST_INTEGER:
            fprintf(output, "%d", AST_INT_VALUE(a));
            break;
        default :
            fprintf(stderr, "Don't know how to print: %d", a->kind);
            exit(1);
    }
}

int ast_cons_length(ast_t *cons)
{
    assert(cons->kind == AST_CONS || cons->kind == AST_END_OF_CONS);
    int i = 0;
    while (cons->kind != AST_END_OF_CONS) {
        i++;
        cons = AST_CONS_CDR(cons);
    }
    return i;
}

int is_valof_form(ast_t *expr)
{
    if (expr->kind == AST_CONS
            && AST_CONS_CAR(expr)->kind == AST_IDENTIFIER
            && strcmp(AST_IDENT_NAME( AST_CONS_CAR(expr) ), "valof") == 0
            && AST_CONS_CADR(expr)->kind == AST_IDENTIFIER)
        return 1;
    else
        return 0;
}
