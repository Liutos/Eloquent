/*
 * prototype.c
 *
 *
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

//lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env);
//lisp_object_t *read_object(lisp_object_t *input_file);
//void write_object(lisp_object_t *object, lisp_object_t *out_port);
//void writef(lt *, const char *, ...);

/* PART: vm_test.c */
/* Driver */
int main(int argc, char *argv[])
{
  char *inputs[] = {
    "(set! abs (lambda (x) (if (> 0 x) (- 0 x) x)))",
    "(abs 1",
    "(abs -1)",
    "#\\a",
    "(code-char 97)",
    "()",
    "(tail '(1))",
//    "#r",
//    "#f",
//    "(> 1 2)",
//    "(= 1 1.0)",
//    "(try-with (/ 1 0) ((devideByZero (ex)) 0))",
  };
//  init_object_pool();
  init_global_variable();
  init_prims();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    lisp_object_t *expr = read_object_from_string(inputs[i]);
    expr = compile_as_lambda(expr);
    writef(standard_out, ">> %s\n", make_string(inputs[i]));
    expr = run_by_llam(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
