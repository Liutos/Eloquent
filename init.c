/*
 * init.c
 *
 *  Created on: 2013年10月20日
 *      Author: liutos
 * This file contains the initialization procedures
 */
#include "object.h"
#include "type.h"
#include "utilities.h"

void init_packages(void) {
  pkgs = make_empty_list();
  pkg_lisp = ensure_package("Lisp");
//  (defpackage "OS"
//    (use "Lisp"))
  pkg_os = ensure_package("OS");
  use_package_in(pkg_lisp, pkg_os);
//  (defpackage "Time"
//    (use "Lisp"))
  pkg_time = ensure_package("Time");
  use_package_in(pkg_lisp, pkg_time);
//  (defpackage "User"
//    (use "Lisp"))
  pkg_user = ensure_package("User");
  use_package_in(pkg_lisp, pkg_user);
  use_package_in(pkg_os, pkg_user);
  use_package_in(pkg_time, pkg_user);
// Set the current package
  package = pkg_lisp;
}

void init_structures(void) {
  st_tbl = make_structures_table();
}

void init_global_variable(void) {
  /* Initialize global variables */
  debug = FALSE;
  is_check_exception = TRUE;
  is_check_type = TRUE;

  the_argv = make_vector(0);
  the_false = make_false();
  the_true = make_true();
  the_empty_list = make_empty_list();
  the_eof = make_eof();
  gensym_counter = make_fixnum(0);
  null_env = make_environment(the_empty_list, NULL);
  environment_next(null_env) = null_env;
  standard_error = make_output_port(stderr);
  standard_in = make_input_port(stdin);
  standard_out = make_output_port(stdout);
  symbol_list = the_empty_list;
  the_undef = make_undef();

//  Character
  init_character();
//  Opcode
  prim2op_map = make_prim2op_map();
  init_opcode_length();
//  Packages initialization
  init_packages();
//  Structures initialization
  init_structures();

// Global variables initialization
  symbol_value(S("*ARGV*")) = the_argv;
  symbol_value(S("*gensym-counter*")) = gensym_counter;
  symbol_value(S("*standard-error*")) = standard_error;
  symbol_value(S("*standard-output*")) = standard_out;
  symbol_value(S("*standard-input*")) = standard_in;

  /* Symbol initialization */
  the_begin_symbol = LISP("begin");
  the_catch_symbol = LISP("catch");
  the_dot_symbol = LISP(".");
  the_goto_symbol = LISP("goto");
  the_if_symbol = LISP("if");
  the_lambda_symbol = LISP("lambda");
  the_quasiquote_symbol = LISP("quasiquote");
  the_quote_symbol = LISP("quote");
  the_set_symbol = LISP("set");
  the_splicing_symbol = LISP("unquote-splicing");
  the_tagbody_symbol = LISP("tagbody");
  the_unquote_symbol = LISP("unquote");
  /* Exception tags initialization */
  the_compiler_error_symbol = LISP("compiler-error");
  the_error_symbol = LISP("error");
  the_reader_error_symbol = LISP("reader-error");
  the_type_error_symbol = LISP("type-error");
}
