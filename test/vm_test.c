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
#include "utilities.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  char *inputs[] = {
      "\"a\"",
      "#\\a",
      "#\\汗",
      "#\\newline",
  };
  init_global_variable();
  init_prims();
  init_primitive_opcode();
  init_macros();
  load_init_file();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    writef(standard_out, "%s >> %s\n", package_name(package), wrap_C_string(inputs[i]));
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
//  char *str = "汉字哦亲";
//  uint32_t cp = get_code_point(str);
//  printf("cp is %d\n", cp);
//  uint32_t *value = utf8s_to_code_point(str);
//  int length = get_string_length(value);
//  printf("length is %d\n", length);
//  printf("value[0] is %d\n", value[0]);
//  printf("value[1] is %d\n", value[1]);
//  str = Lisp_C_string(value, length);
//  printf("str is %s\n", str);
//  return 0;
}
