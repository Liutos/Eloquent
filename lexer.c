/*
 * lexer.c
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils/string.h"

/* PRIVATE */

static int lexer_getc(lexer_t *l)
{
    int c = fgetc(l->src);
    if (c == '\n') {
        l->line++;
        l->column = 0;
    } else
        l->column++;
    return c;
}

static int lexer_ungetc(lexer_t *l, int c)
{
    if (c == '\n') {
        l->line--;
        /* XXX: 此处l->column应当设置为上一行的长度 */
    } else
        l->column--;
    return ungetc(c, l->src);
}

static int lexer_issep(int c)
{
    return isspace(c) || c == EOF || c == '(' || c == ')';
}

static void lexer_skip_comment(lexer_t *lexer)
{
    int c;
    do {
        c = lexer_getc(lexer);
    } while (c != EOF && c != '\n');
}

static token_t lexer_getidentifier(lexer_t *l, int start)
{
    l->tk_line = l->line;
    l->tk_column = l->column;

    string_clear(l->text);
    int c = start;
    do {
        string_addc(l->text, c);
        c = lexer_getc(l);
    } while (!lexer_issep(c));
    lexer_ungetc(l, c);
    return TOKEN_IDENTIFIER;
}

static token_t lexer_getinteger(lexer_t *l, int start)
{
    l->tk_line = l->line;
    l->tk_column = l->column;

    string_clear(l->text);
    int c = start;
    do {
        string_addc(l->text, c);
        c = lexer_getc(l);
    } while (isdigit(c));
    lexer_ungetc(l, c);
    return TOKEN_INTEGER;
}

/* PUBLIC */

lexer_t *lexer_new(FILE *src)
{
    lexer_t *l = malloc(sizeof(lexer_t));
    l->src = src;
    l->text = string_new();
    l->tk_line = l->line = 1;
    l->tk_column = l->column = 0;
    return l;
}

void lexer_free(lexer_t *l)
{
    string_free(l->text);
    free(l);
}

token_t lexer_gettoken(lexer_t *l)
{
    int c = lexer_getc(l);
    switch (c) {
        case EOF: return TOKEN_EOF;
        case ' ': case '\n': case '\t': return lexer_gettoken(l);
        case '(': return TOKEN_LEFT_PAREN;
        case ')': return TOKEN_RIGHT_PAREN;
        case ';':
            lexer_skip_comment(l);
            return lexer_gettoken(l);
        default :
            if (isdigit(c))
                return lexer_getinteger(l, c);
            else
                return lexer_getidentifier(l, c);
    }
}
