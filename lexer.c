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
    return fgetc(l->src);
}

static int lexer_ungetc(lexer_t *l, int c)
{
    return ungetc(c, l->src);
}

static int lexer_issep(int c)
{
    return isspace(c) || c == EOF || c == '(' || c == ')';
}

static token_t lexer_getidentifier(lexer_t *l, int start)
{
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
        default :
            if (isdigit(c))
                return lexer_getinteger(l, c);
            else
                return lexer_getidentifier(l, c);
    }
}
