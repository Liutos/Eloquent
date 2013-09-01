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
  };
  init_global_variable();
  init_prims();
  init_compiled_prims();
  init_macros();
  char *load_expr = "(load \"init.scm\")";
  lt *expr = read_object_from_string(strdup(load_expr));
//  write_raw_string(load_expr, standard_out);
  writef(standard_out, "%s\n", make_string(load_expr));
  expr = compile_to_bytecode(expr);
  if (!is_signaled(expr))
    expr = run_by_llam(expr);
  if (is_signaled(expr))
    writef(standard_out, "%?\n", expr);
  else
    writef(standard_out, "=> %?\n", expr);
//  load_init_file();
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
