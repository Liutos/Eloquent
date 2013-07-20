/*
 * compiler_test.c
 *
 *  Created on: 2013年7月19日
 *      Author: liutos
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
  char *inputs[] = {
  		"123",
  		"-123",
  		"1.23",
  		"#\\a",
  		"\"Hello, world!\"",
  		"(+ 1 2)",
  		"'abc",
  		"(quote abc)",
  		"[1 2 3 4]",
  		"(begin (+ 1 2) 1)",
  		"(lambda (x) (+ x 1))",
  		"(if #t 1 2)",
  		"(set! abc 'abc)",
  };
  init_global_variable();
  init_prims();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    write_raw_string(">> ", standard_out);
    write_raw_string(inputs[i], standard_out);
    write_raw_char('\n', standard_out);
    lisp_object_t *expr = read_object_from_string(inputs[i]);
//    expr = compile_as_lambda(expr);
    expr = compile_object(expr, null_env);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
