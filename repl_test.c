/*
 * repl_test.c
 *
 *  Created on: 2013年7月26日
 *      Author: liutos
 */
#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "prims.h"
#include "type.h"
#include "vm.h"

int main(int argc, char *argv[]) {
  init_global_variable();
  init_prims();
  init_macros();
  load_init_file();
  while (1) {
    write_raw_string(">> ", standard_out);
    lt *expr = read_object(standard_in);
    expr = compile_to_bytecode(expr);
    expr = run_by_llam(expr);
    if (iseof(expr)) {
      write_raw_string("Goodbye!\n", standard_out);
      break;
    }
    write_raw_string("=> ", standard_out);
    write_object(expr, standard_out);
    write_raw_char('\n', standard_out);
  }
  return 0;
}
