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
extern lt *make_output_port(FILE *);
extern lt *make_output_string_port(char *);
extern lt *make_package(lt *, hash_table_t *);
extern lt *make_pair(lt *, lt *);
extern lt *make_primitive(int, void *, char *, int);
extern lt *make_retaddr(lt *code, lt *env, lt *fn, int pc, int throw_flag, int sp);
extern string_builder_t *make_str_builder(void);
extern lt *make_string(char *);
extern lt *make_structure(lt *name, int nfield);
extern lt *make_symbol(char *, lt *);
extern lt *make_time(struct tm *);
extern lt *make_type(enum TYPE, char *);
extern lt *make_unicode(char *);
extern lt *make_vector(int);
extern lt *make_op_call(lt *);
extern lt *make_op_checkex(void);
extern lt *make_op_chkarity(lt *);
extern lt *make_op_chktype(lt *, lt *, lt *);
extern lt *make_op_const(lt *);
extern lt *make_op_extenv(lt *);
extern lt *make_op_fjump(lt *);
extern lt *make_op_fn(lt *);
extern lt *make_op_gset(lt *);
extern lt *make_op_gvar(lt *);
extern lt *make_op_jump(lt *);
extern lt *make_op_lset(lt *i, lt *j, lt *symbol);
extern lt *make_op_lvar(lt *i, lt *j, lt *symbol);
extern lt *make_op_moveargs(lt *);
extern lt *make_op_pop(void);
extern lt *make_op_popenv(void);
extern lt *make_op_prim(lt *);
extern lt *make_op_restargs(lt *);
extern lt *make_op_return(void);
extern lt *make_op_catch(void);
extern lt *make_fn_inst(lt *);

extern lt *opcode_ref(enum OPCODE_TYPE);
extern lt *search_op4prim(lt *);
extern void set_op4prim(lt *, enum OPCODE_TYPE);
/* Structure */
extern lt *search_structure(char *);
extern void set_structure(char *, lt *);
/* Symbol */
extern hash_table_t *make_symbol_table(void);
extern lt *find_or_create_symbol(char *, lt *);
/* Package */
extern lt *ensure_package(char *);
extern lt *search_package(char *, lt *);
extern lt *type_ref(enum TYPE);

extern void init_global_variable(void);

/* tagging system
 *   bits end in  00:  pointer
 *                01:  fixnum
 *              0110:  byte
 *              1110:  other immediate object (null_list, true, false, eof, undef, close)
 */
#define BYTE_BITS 4
#define BYTE_MASK 15
#define BYTE_TAG 6
#define FIXNUM_BITS 2
#define FIXNUM_MASK 3
#define FIXNUM_TAG 1
#define IMMEDIATE_BITS 4
#define IMMEDIATE_MASK 15
#define IMMEDIATE_TAG 14
#define POINTER_MASK 3
#define POINTER_TAG 0

#define LISP(name) find_or_create_symbol(name, pkg_lisp)
#define S(name) (find_or_create_symbol(name, package))

#endif
