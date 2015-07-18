#include <stdio.h>
#include "ast.h"
#include "interp.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    lexer_t *lexer = lexer_new(stdin);
    parser_t *parser = parser_new(lexer);
    interp_t *interp = interp_new();
    ast_t *ast = NULL;
    ast_kind_t kind = parser_getast(parser, &ast);
    while (kind != AST_INVALID) {
        if (kind == AST_END_OF_FILE)
            break;
        ast_print(ast, stdout);
        fprintf(stdout, "\n");
        value_t *value = NULL;
        if (interp_execute(interp, ast, &value) != VALUE_INVALID) {
            value_print(value, stdout);
            fprintf(stdout, "\n");
        } else {
            fprintf(stdout, "%s\n", value->u.invalid_msg->text);
        }
        kind = parser_getast(parser, &ast);
    }
    return 0;
}
