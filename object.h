/*
 * object.h
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */

#ifndef OBJECT_H_
#define OBJECT_H_

/* OBJECT_H_ */
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <gmp.h>

#include "type.h"

int debug;
int is_check_exception;
int is_check_type;
/* Opcode */
int opcode_max_length;
hash_table_t *prim2op_map;
/* Structure */
hash_table_t *st_tbl;
/* Symbol */
/** Special Forms **/
lt *the_begin_symbol;
lt *the_catch_symbol;
lt *the_dot_symbol;
lt *the_goto_symbol;
lt *the_if_symbol;
lt *the_lambda_symbol;
lt *the_quasiquote_symbol;
lt *the_quote_symbol;
lt *the_set_symbol;
lt *the_splicing_symbol;
lt *the_tagbody_symbol;
lt *the_unquote_symbol;
/** Exceptions **/
lt *the_compiler_error_symbol;
lt *the_error_symbol;
lt *the_reader_error_symbol;
lt *the_type_error_symbol;

lt *the_argv;
lisp_object_t *the_false;
lisp_object_t *the_true;
lt *gensym_counter;
lisp_object_t *null_env;
lisp_object_t *the_empty_list;
lt *the_eof;
/* Package */
lt *package;
lt *pkg_lisp;
lt *pkg_os;
lt *pkg_time;
lt *pkg_user;
lt *pkgs;

lt *standard_error;
lisp_object_t *standard_in;
lisp_object_t *standard_out;
lisp_object_t *symbol_list;
lisp_object_t *the_undef;

extern int is_pointer(lt *);
extern int is_lt_bignum(lt *);
extern int is_lt_environment(lt *);
extern int is_lt_exception(lt *);
extern int is_lt_float(lt *);
extern int is_lt_function(lt *);
extern int is_lt_input_port(lt *);
extern int is_lt_mpflonum(lt *);
extern int is_lt_opcode(lt *);
extern int is_lt_output_port(lt *);
extern int is_lt_pair(lt *);
extern int is_lt_primitive(lt *);
extern int is_lt_string(lt *);
extern int is_lt_symbol(lt *);
extern int is_lt_type(lt *);
extern int is_lt_vector(lt *);
extern int is_lt_unicode(lt *);
extern int is_lt_byte(lt *);
extern int isfixnum(lt *);
extern int isdot(lt *);
extern int iseof(lt *);
extern int isnull(lt *);
extern int isfalse(lt *);
extern int is_true_object(lt *);
extern int isundef(lt *);
extern int isclose(lt *);
extern int isboolean(lt *);
extern int is_signaled(lt *);
extern int isnull_env(lt *);
extern int isnumber(lt *);
extern int isopcode_fn(lt *);
extern int type_of(lt *);
// Hash Table
extern hash_table_t *make_hash_table(int, hash_fn_t, comp_fn_t);
// Constructors
extern lt *make_false(void);
extern lt *make_true(void);
extern lt *make_empty_list(void);
extern lt *make_eof(void);
extern lt *make_undef(void);
extern lt *make_close(void);
extern lt *make_byte(char);
extern lt *make_fixnum(int);
extern lt *make_bignum(mpz_t);
extern lt *make_environment(lt *, lt *);
extern lt *make_exception(char *, int, lt *, lt *backtrace);
extern lt *make_float(float);
extern lt *make_function(lt *cenv, lt *args, lt *code, lt *renv);
extern lt *make_input_port(FILE *);
extern lt *make_input_string_port(char *);
extern lt *make_mpflonum(mpf_t);
extern lt *make_opcode(enum OPCODE_TYPE, char *, lt *);
extern lt *make_output_port(FILE *);
extern lt *make_output_string_port(char *);
extern lt *make_package(lt *, hash_table_t *);
extern lt *make_pair(lt *, lt *);
extern lt *make_primitive(int, void *, char *, int);
extern lt *make_retaddr(lt *code, lt *env, lt *fn, int pc, int throw_flag, int sp);
extern lt *make_string(int, uint32_t *);
extern lt *make_structure(lt *name, int nfield);
extern lt *make_symbol(char *, lt *);
extern lt *make_time(struct tm *);
extern lt *make_type(enum TYPE, char *);
extern lt *make_unicode(char *);
extern lt *make_vector(int);

/* Opcode */
extern void init_opcode_length(void);
extern lt *opcode_ref(enum OPCODE_TYPE);
/* Type */
extern lt *type_ref(enum TYPE);
/* Unicode */
extern void init_character(void);
extern lt *make_unicode_char(char);

#endif
