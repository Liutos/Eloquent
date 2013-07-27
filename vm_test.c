/*
 * prototype.c
 *
 * A Simple Byte Code Compiler for Lisp
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
      "(expand-macro '(cond))",
      "(cond)",
      "(expand-macro '(cond (1 (+ 1 2))))",
      "(cond (1 (+ 1 2)))",
      "(expand-macro '(cond (#f (/ 1 0)) (else (+ 1 2.3))))",
      "(cond (#f (/ 1 0)) (else (+ 1 2.3)))",
      "(expand-macro '(cond (#f (/ 1 0)) ((tail '(1)) (/ 2 2)) (else (+ 2.3 1))))",
      "(cond (#f (/ 1 0)) ((tail '(1)) (/ 2 2)) (else (+ 2.3 1)))",
  };
  init_global_variable();
  init_prims();
  init_macros();
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
