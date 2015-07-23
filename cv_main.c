/*
 * cv_main.c
 *
 *  Created on: 2015年7月21日
 *      Author: liutos
 */
#include <stdio.h>
#include "ast.h"
#include "bytecode.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"

static ast_kind_t prompt(parser_t *parser, ast_t **ast)
{
    fprintf(stdout, "233-LISP > ");
    fflush(stdout);
    return parser_getast(parser, ast);
}

int main(int argc, char *argv[])
{
    lexer_t *lexer = lexer_new(stdin);
    parser_t *parser = parser_new(lexer);
    compiler_t *comp = compiler_new();
    vm_t *vm = vm_new();
    ast_t *ast = NULL;
    ast_kind_t kind = prompt(parser, &ast);
    while (kind != AST_INVALID) {
        if (kind == AST_END_OF_FILE)
            break;
        fprintf(stdout, "AST > ");
        fflush(stdout);
        ast_print(ast, stdout);
        fputc('\n', stdout);

        ins_t *ins = ins_new();
        if (compiler_do(comp, ast, ins) == 1) {
            fprintf(stdout, "BYTECODE >\n");
            fflush(stdout);
            ins_pretty_print(ins, stdout);

            vm_execute(vm, ins);
            fprintf(stdout, "VM > ");
            fflush(stdout);
            vm_print_top(vm, stdout);
        } else
            fprintf(stdout, "Compiler error: %s\n", comp->error->text);
        kind = prompt(parser, &ast);
    }
    return 0;
}
