/*
 * lexer.h
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>
#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __lexer_t lexer_t;

typedef enum {
    TOKEN_INVALID,
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
} token_t;

struct __lexer_t {
    FILE *src;
    string_t *text;
    int line, column;
    int tk_line, tk_column;
};

extern lexer_t *lexer_new(FILE *);
extern void lexer_free(lexer_t *);
extern token_t lexer_gettoken(lexer_t *);

#ifdef __cplusplus
}
#endif

#endif /* LEXER_H_ */
