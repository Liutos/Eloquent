#include <stdio.h>
#include "ast.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    lexer_t *lexer = lexer_new(stdin);
    parser_t *parser = parser_new(lexer);
    ast_t *ast = NULL;
    ast_kind_t kind = parser_getast(parser, &ast);
    while (kind != AST_INVALID) {
        if (kind == AST_END_OF_FILE)
            break;
        ast_print(ast, stdout);
        fprintf(stdout, "\n");
        kind = parser_getast(parser, &ast);
    }
    return 0;
}
