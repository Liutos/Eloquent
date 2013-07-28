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
      "(expand-macro '(var a 1))",
      "((lambda (x) (var a 1) (+ a x)) 2)",
      "(set! mk-adder (lambda (x) (lambda (n) (var m n) (+ m x))))",
      "(set! add1 (mk-adder 1))",
      "(add1 2)",
      "(let ((a 1)) (+ a 1))",
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
