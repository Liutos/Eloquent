#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __parser_t parser_t;

struct __parser_t {
    lexer_t *lexer;
};

extern parser_t *parser_new(lexer_t *);
extern void parser_free(parser_t *);
extern ast_kind_t parser_getast(parser_t *, ast_t **);

#ifdef __cplusplus
}
#endif

#endif /* PARSER_H_ */
