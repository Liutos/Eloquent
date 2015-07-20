#include <stdio.h>
#include "ast.h"
#include "bytecode.h"
#include "compiler.h"
#include "interp.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    lexer_t *lexer = lexer_new(stdin);
    parser_t *parser = parser_new(lexer);
    interp_t *interp = interp_new();
    compiler_t *comp = compiler_new();
    ast_t *ast = NULL;
    ast_kind_t kind = parser_getast(parser, &ast);
    while (kind != AST_INVALID) {
        if (kind == AST_END_OF_FILE)
            break;
        ast_print(ast, stdout);
        fprintf(stdout, "\n");

        ins_t *ins = ins_new();
        if (compiler_do(comp, ast, ins) == 1) {
            ins_print(ins, stdout);
        }

        value_t *value = NULL;
        interp_execute(interp, ast, &value);
        value_print(value, stdout);
        fprintf(stdout, "\n");
        kind = parser_getast(parser, &ast);
    }
    return 0;
}
