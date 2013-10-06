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
      "(make-bignum \"9876543210123456789\")",
      "(set! n (make-bignum \"1\"))",
      "(set! m (make-bignum \"2\"))",
      "(bg+ n m)",
      "(bg- n m)",
      "(bg* n m)",
      "(bg/ n m)",
      "(type-of n)",
      "(fx->bg 1)",
      "(set! n (nt-convert 1 'fixnum 'flonum))",
      "(set! m (nt-convert 1 'fixnum 'bignum))",
      "(type-of n)",
      "(type-of m)",
      "(bg= (make-bignum \"1\") (make-bignum \"1\"))",
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
