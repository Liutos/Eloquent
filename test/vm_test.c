/*
 * vm_test.c
 *
 * The Test Driver for the Virtual Machine
 *
 * Copyright (C) 2013-06-07 liutos <mat.liutos@gmail.com>
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
      "(dolist (i '(#\\a #\\b #\\c)) (write-char i *standard-output*) (write-char #\\newline *standard-output*))",
      "(dotimes (i 3) (write-object i *standard-output*) (write-char #\\newline *standard-output*))",
  };
  init_global_variable();
  init_prims();
  init_primitive_opcode();
  init_macros();
  load_init_file();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    writef(standard_out, "%s >> %s\n", package_name(package), make_string(inputs[i]));
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
