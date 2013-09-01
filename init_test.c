/*
 * init_test.c
 *
 *  Created on: 2013年9月1日
 *      Author: liutos
 */
#include <string.h>

#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "prims.h"
#include "type.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  char *inputs[] = {
      "(< 1 2)",
      "(< 1 1.0)",
      "(< -1 0)",
      "(bin+ 1 1)",
      "(bin+ 1 1.0)",
      "(bin+ 1.0 1)",
      "(bin+ 1.0 1.0)",
      "(bin- 1 2)",
      "(bin- 1 -1.0)",
      "(bin- 1.0 2)",
      "(bin- 2.0 2.1)",
      "(bin* 0 2)",
      "(bin* 0 2.0)",
      "(bin* 0.0 1)",
      "(bin* 2.0 3.0)",
      "(bin/ 1 2)",
      "(bin/ 1 2.0)",
      "(bin/ 1.0 2)",
      "(bin/ 1.0 2.0)",
      "(bin/ 1 0)",
      "(null? '())",
      "(null? 1)",
      "(null? '(1))",
      "(first '(1 2 3))",
      "(rest '(1 2 3))",
      "(reduce '() bin+)",
      "(reduce '(1) bin+)",
      "(reduce '(1 2 3 4) bin+)",
      "(+)",
      "(+ 1)",
      "(+ 1 2 3 4 5 6)",
      "(- 1)",
      "(- 1 2 3)",
      "(* 1 2)",
      "(* 1 2 3 4)",
      "(/ 1 2)",
      "(/ 1 2.0 4.0)",
  };
  init_global_variable();
  init_prims();
  init_compiled_prims();
  init_macros();
  char *load_expr = "(load \"init.scm\")";
  lt *expr = read_object_from_string(strdup(load_expr));
  writef(standard_out, "%s\n", make_string(load_expr));
  expr = compile_to_bytecode(expr);
  if (!is_signaled(expr))
    expr = run_by_llam(expr);
  if (is_signaled(expr))
    writef(standard_out, "%?\n", expr);
  else
    writef(standard_out, "=> %?\n", expr);
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    writef(standard_out, ">> %s\n", make_string(inputs[i]));
    lisp_object_t *expr = read_object_from_string(strdup(inputs[i]));
    expr = compile_to_bytecode(expr);
    if (!is_signaled(expr))
      expr = run_by_llam(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
