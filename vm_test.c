/*
 * prototype.c
 *
 * A Simple Byte Code Compiler for Lisp
 *
 * Copyright (C) 2013-06-07 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "compiler.h"
#include "vm.h"
#include "object.h"
#include "prims.h"

int main(int argc, char *argv[])
{
  lt *assemble(lt *);
  char *inputs[] = {
      "((lambda (x y) (if #t x y)) 1 2)",
      "((lambda (x) ((lambda (y) (+ x y)) 2)) 1)",
  };
  init_global_variable();
  init_prims();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    writef(standard_out, ">> %s\n", make_string(inputs[i]));
    lisp_object_t *expr = read_object_from_string(inputs[i]);
//    expr = compile_as_lambda(expr);
    expr = compile_object(expr, null_env);
    expr = assemble(expr);
    expr = run_by_llam(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
