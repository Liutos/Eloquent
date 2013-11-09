/*
 * compiler_test.c
 *
 *  Created on: 2013年7月19日
 *      Author: liutos
 */
#include <string.h>

#include "compiler.h"
#include "init.h"
#include "macros.h"
#include "object.h"
#include "prims.h"
#include "type.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  char *inputs[] = {
      "(quote x)",
      "(let ((a 1) (b 2)) (cons a b))",
      "(begin 1 (+ 1 1) 3)",
      "(set! a '(1 2 3))",
      "(if (= n 0) 1 (* n (- n 1)))",
      "(lambda (x y) (+ x y))",
      "(multiple-value-list (+ 1 2 3))",
      "(catch)",
      "(return 1)",
      "(tagbody a (goto b) b 1)",
      "(values 1 2 3)",
      "((lambda (x . y) (values x y)))",
  };
  init_global_variable();
  init_prims();
  init_primitive_opcode();
  init_macros();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    write_raw_string(">> ", standard_out);
    write_raw_string(inputs[i], standard_out);
    write_raw_char('\n', standard_out);
    lisp_object_t *expr = read_object_from_string(strdup(inputs[i]));
    expr = compile_to_bytecode(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
