#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lexer.h"
#include "parser.h"

/* PRIVATE */

static void parser_posast(parser_t *parser, ast_t *ast)
{
    ast->line = parser->lexer->tk_line;
    ast->column = parser->lexer->tk_column;
}

static ast_kind_t parser_getcons(parser_t *parser, ast_t **ast)
{
    int line = parser->lexer->tk_line;
    int column = parser->lexer->tk_column;

    ast_t *car = NULL;
    ast_kind_t kind = parser_getast(parser, &car);
    if (kind == AST_INVALID) {
        if (ast != NULL)
            *ast = car;
        return AST_INVALID;
    }
    if (kind == AST_RIGHT_PAREN) {
        if (ast != NULL)
            *ast = ast_eoc_new();
        return AST_END_OF_CONS;
    }
    ast_t *cdr = NULL;
    if (parser_getcons(parser, &cdr) == AST_INVALID) {
        if (ast != NULL)
            *ast = cdr;
        return AST_INVALID;
    }
    if (ast != NULL) {
        *ast = ast_cons_new(car, cdr);
        (*ast)->line = line;
        (*ast)->column = column;
    }
    return AST_CONS;
}

static ast_kind_t parser_getidentifier(parser_t *parser, ast_t **ast)
{
    lexer_t *lexer = parser->lexer;
    const char *text = lexer->text->text;
    if (ast != NULL) {
        *ast = ast_ident_new(text);
        parser_posast(parser, *ast);
    }
    return AST_IDENTIFIER;
}

static ast_kind_t parser_getinteger(parser_t *parser, ast_t **ast)
{
    lexer_t *lexer = parser->lexer;
    const char *text = lexer->text->text;
    if (ast != NULL) {
        *ast = ast_int_new(atoi(text));
        parser_posast(parser, *ast);
    }
    return AST_INTEGER;
}

/* PUBLIC */

parser_t *parser_new(lexer_t *l)
{
    parser_t *p = malloc(sizeof(parser_t));
    p->lexer = l;
    return p;
}

void parser_free(parser_t *p)
{
    free(p);
}

ast_kind_t parser_getast(parser_t *parser, ast_t **ast)
{
    lexer_t *l = parser->lexer;
    token_t token = lexer_gettoken(l);
    switch (token) {
        case TOKEN_EOF:
            return AST_END_OF_FILE;
        case TOKEN_IDENTIFIER:
            return parser_getidentifier(parser, ast);
        case TOKEN_INTEGER:
            return parser_getinteger(parser, ast);
        case TOKEN_LEFT_PAREN:
            return parser_getcons(parser, ast);
        case TOKEN_RIGHT_PAREN:
            return AST_RIGHT_PAREN;
        case TOKEN_INVALID:
            if (ast != NULL)
                *ast = ast_invalid_new();
            return AST_INVALID;
        default :
            fprintf(stderr, "Unknown token: %d", token);
            exit(1);
    }
}
