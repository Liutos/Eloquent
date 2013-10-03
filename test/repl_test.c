/*
 * repl_test.c
 *
 *  Created on: 2013年7月26日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "prims.h"
#include "type.h"
#include "vm.h"

int main(int argc, char *argv[]) {
  int script_flag = FALSE;
  lt *script_name = the_undef;
  init_global_variable();
  init_prims();
  init_primitive_opcode();
  init_macros();
  load_init_file();

  int opt;
  while ((opt = getopt(argc, argv, "l:")) != -1) {
    switch (opt) {
      case 'l': {
        script_flag = TRUE;
        char *script = optarg;
        script_name = make_string(script);
      }
        break;
      default :
        fprintf(stderr, "Unknown command line option %c\n", opt);
        exit(1);
    }
  }

//  Initializes the command line arguments array
  for (int i = 0; i < argc; i++) {
    lt_vector_push_extend(the_argv, make_string(argv[i]));
  }

  if (script_flag == TRUE) {
    lt_load(script_name);
    exit(0);
  }

  while (1) {
    writef(standard_out, "%s >> ", package_name(package));
    lt *expr = read_object(standard_in);
    expr = compile_to_bytecode(expr);
    if (!is_signaled(expr))
      expr = run_by_llam(expr);
    if (iseof(expr)) {
      write_raw_string("Goodbye!\n", standard_out);
      break;
    }
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
