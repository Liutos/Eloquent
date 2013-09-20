/*
 * compiler_test.c
 *
 *  Created on: 2013年7月19日
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
      "(cons 1 2)",
  };
  init_global_variable();
  init_prims();
  init_primitive_opcode();
  init_compiled_prims();
  init_macros();
  load_init_file();
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
