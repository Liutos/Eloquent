/*
 * vm_test.c
 *
 * The Test Driver for the Virtual Machine
 *
 * Copyright (C) 2013-06-07 liutos <mat.liutos@gmail.com>
 */
#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "prims.h"
#include "type.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  char *inputs[] = {
      "(read-tokens-from-string \"1 2.3 #t #\\\\a\")",
      "(read-tokens-from-string \"lambda (x) x + 1\")",
      "(read-tokens-from-string \"if x < 0 0 - x else x\")",
      "(read-tokens-from-string \"func([1, 2.3, 4.56, (x + y)])\")",
  };
  init_global_variable();
  init_prims();
  init_macros();
  load_init_file();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    writef(standard_out, ">> %s\n", make_string(inputs[i]));
    lisp_object_t *expr = read_object_from_string(inputs[i]);
    expr = compile_to_bytecode(expr);
    expr = run_by_llam(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
