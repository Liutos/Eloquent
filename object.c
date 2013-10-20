/*
 * object.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 *
 * This file contains
 * |-- The constructor functions of all Lisp types
 * `-- The type predicate functions of all Lisp types
 */
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gc/gc.h>
#include <gmp.h>

#include "hash_table.h"
#include "object.h"
#include "type.h"

int debug;
int is_check_exception;
int is_check_type;
char tbl[256];
lt *gensym_counter;
lt *null_env;
/* Opcode */
int opcode_max_length;
hash_table_t *prim2op_map;
/* Package */
lt *package;
lt *pkg_lisp;
lt *pkg_os;
lt *pkg_time;
lt *pkg_user;
lt *pkgs;

lt *standard_error;
lt *standard_in;
lt *standard_out;
lt *symbol_list;
/* Structure */
// The global variable `st_tbl' contains the mapping between `name of structure' and `fields of structure'
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
lt *the_empty_list;
lt *the_eof;
lt *the_false;
lt *the_true;
lt *the_undef;

#define DEFTYPE(tag, name) {.type=LT_TYPE, .u={.type={tag, name}}}

struct lisp_object_t lt_types[] = {
    DEFTYPE(LT_BOOL, "bool"),
    DEFTYPE(LT_BYTE, "byte"),
    DEFTYPE(LT_EMPTY_LIST, "empty-list"),
    DEFTYPE(LT_FIXNUM, "fixnum"),
    DEFTYPE(LT_TCLOSE, "tclose"),
    DEFTYPE(LT_TEOF, "teof"),
    DEFTYPE(LT_TUNDEF, "tundef"),
    DEFTYPE(LT_BIGNUM, "bignum"),
    DEFTYPE(LT_ENVIRONMENT, "environment"),
    DEFTYPE(LT_EXCEPTION, "exception"),
    DEFTYPE(LT_FUNCTION, "function"),
    DEFTYPE(LT_FLOAT, "float"),
    DEFTYPE(LT_INPUT_PORT, "input-file"),
    DEFTYPE(LT_MPFLONUM, "mpflonum"),
    DEFTYPE(LT_OPCODE, "opcode"),
    DEFTYPE(LT_OUTPUT_PORT, "output-file"),
    DEFTYPE(LT_PACKAGE, "package"),
    DEFTYPE(LT_PAIR, "pair"),
    DEFTYPE(LT_PRIMITIVE, "primitive"),
    DEFTYPE(LT_RETADDR, "retaddr"),
    DEFTYPE(LT_STRING, "string"),
    DEFTYPE(LT_STRUCT, "structure"),
    DEFTYPE(LT_SYMBOL, "symbol"),
    DEFTYPE(LT_TIME, "time"),
    DEFTYPE(LT_TYPE, "type"),
    DEFTYPE(LT_UNICODE, "unicode"),
    DEFTYPE(LT_VECTOR, "vector"),
};

#define DEFCODE(name) {.type=LT_OPCODE, .u={.opcode={name, #name}}}

struct lisp_object_t lt_codes[] = {
    DEFCODE(CALL),
    DEFCODE(CATCH),
    DEFCODE(CHECKEX),
    DEFCODE(CHKARITY),
    DEFCODE(CHKTYPE),
    DEFCODE(CONST),
    DEFCODE(EXTENV),
    DEFCODE(FN),
    DEFCODE(GSET),
    DEFCODE(GVAR),
    DEFCODE(FJUMP),
    DEFCODE(JUMP),
    DEFCODE(LSET),
    DEFCODE(LVAR),
    DEFCODE(MOVEARGS),
    DEFCODE(POP),
    DEFCODE(POPENV),
    DEFCODE(PRIM),
    DEFCODE(RESTARGS),
    DEFCODE(RETURN),
//    Opcodes for some primitive functions
    DEFCODE(ADDI),
    DEFCODE(CONS),
    DEFCODE(DIVI),
    DEFCODE(MULI),
    DEFCODE(SUBI),
};

/* Type predicate */
int is_lt_byte(lt *object) {
  return ((intptr_t)object & BYTE_MASK) == BYTE_TAG;
}

int isfixnum(lt *object) {
  return ((intptr_t)object & FIXNUM_MASK) == FIXNUM_TAG;
}

int is_pointer(lt *object) {
  return ((intptr_t)object & POINTER_MASK) == POINTER_TAG;
}

int is_of_type(lisp_object_t *object, enum TYPE type) {
  return is_pointer(object) && (object->type == type? TRUE: FALSE);
}

#define mktype_pred(func_name, type)            \
  int func_name(lisp_object_t *object) {        \
    return is_of_type(object, type);            \
  }

mktype_pred(is_lt_bignum, LT_BIGNUM)
mktype_pred(is_lt_environment, LT_ENVIRONMENT)
mktype_pred(is_lt_exception, LT_EXCEPTION)
mktype_pred(is_lt_float, LT_FLOAT)
mktype_pred(is_lt_function, LT_FUNCTION)
mktype_pred(is_lt_input_port, LT_INPUT_PORT)
mktype_pred(is_lt_mpflonum, LT_MPFLONUM)
mktype_pred(is_lt_output_port, LT_OUTPUT_PORT)
mktype_pred(is_lt_opcode, LT_OPCODE)
mktype_pred(is_lt_pair, LT_PAIR)
mktype_pred(is_lt_primitive, LT_PRIMITIVE)
mktype_pred(is_lt_string, LT_STRING)
mktype_pred(is_lt_symbol, LT_SYMBOL)
mktype_pred(is_lt_type, LT_TYPE)
mktype_pred(is_lt_unicode, LT_UNICODE)
mktype_pred(is_lt_vector, LT_VECTOR)

int is_immediate(lt *object) {
  return ((intptr_t)object & IMMEDIATE_MASK) == IMMEDIATE_TAG;
}

int is_tag_immediate(lt *object, int origin) {
  return is_immediate(object) && ((intptr_t)object >> IMMEDIATE_BITS) == origin;
}

#define mkim_pred(func_name, origin)		      \
  int func_name(lt *object) {			            \
	  return is_tag_immediate(object, origin);	\
  }

mkim_pred(iseof, EOF_ORIGIN)
mkim_pred(isnull, NULL_ORIGIN)
mkim_pred(isfalse, FALSE_ORIGIN)
mkim_pred(is_true_object, TRUE_ORIGIN)
mkim_pred(isundef, UNDEF_ORIGIN)
mkim_pred(isclose, CLOSE_ORIGIN)

int isboolean(lisp_object_t *object) {
  return isfalse(object) || is_true_object(object);
}

int is_signaled(lisp_object_t *object) {
  return is_lt_exception(object) && exception_flag(object) == TRUE;
}

int isdot(lisp_object_t *object) {
  return object == the_dot_symbol;
}

int isnull_env(lt *obj) {
  return obj == null_env;
}

int isnumber(lisp_object_t *object) {
  return isfixnum(object) || is_lt_float(object);
}

int type_of(lisp_object_t *x) {
  if (isboolean(x))
    return LT_BOOL;
  if (is_lt_byte(x))
    return LT_BYTE;
  if (isnull(x))
    return LT_EMPTY_LIST;
  if (isfixnum(x))
    return LT_FIXNUM;
  if (isclose(x))
    return LT_TCLOSE;
  if (iseof(x))
    return LT_TEOF;
  if (isundef(x))
    return LT_TUNDEF;
  assert(is_pointer(x));
  return x->type;
}

/* Constructor functions */
lt *allocate_object(void) {
  return GC_MALLOC(sizeof(struct lisp_object_t));
}

lisp_object_t *make_object(enum TYPE type) {
  lt *obj = allocate_object();
  obj->type = type;
  return obj;
}

#define MAKE_IMMEDIATE(origin) \
  ((lt *)(((intptr_t)origin << IMMEDIATE_BITS) | IMMEDIATE_TAG))

#define mksingle_type(func_name, origin)    \
  lt *func_name(void) {             \
    return MAKE_IMMEDIATE(origin);      \
  }

mksingle_type(make_false, FALSE_ORIGIN)
mksingle_type(make_true, TRUE_ORIGIN)
mksingle_type(make_empty_list, NULL_ORIGIN)
mksingle_type(make_eof, EOF_ORIGIN)
mksingle_type(make_undef, UNDEF_ORIGIN)
mksingle_type(make_close, CLOSE_ORIGIN)

lisp_object_t *make_byte(char value) {
  return (lt *)((((intptr_t)value) << BYTE_BITS) | BYTE_TAG);
}

lt *make_fixnum(int value) {
  return (lt *)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lt *make_bignum(mpz_t value) {
  lt *obj = make_object(LT_BIGNUM);
  *bignum_value(obj) = *value;
  return obj;
}

lt *make_environment(lt *bindings, lt *next) {
  lt *env = make_object(LT_ENVIRONMENT);
  environment_bindings(env) = bindings;
  environment_next(env) = next;
  return env;
}

lt *make_exception(char *message, int signal_flag, lt *tag, lt *backtrace) {
  lt *ex = make_object(LT_EXCEPTION);
  exception_msg(ex) = message;
  exception_flag(ex) = signal_flag;
  exception_backtrace(ex) = backtrace;
  exception_tag(ex) = tag;
  return ex;
}

lisp_object_t *make_float(float value) {
  lisp_object_t *flt_num = make_object(LT_FLOAT);
  float_value(flt_num) = value;
  return flt_num;
}

lt *make_function(lt *cenv, lt *args, lt *code, lt *renv) {
  lt *func = make_object(LT_FUNCTION);
  function_cenv(func) = cenv;
  function_args(func) = args;
  function_code(func) = code;
  function_name(func) = the_undef;
  function_renv(func) = renv;
  return func;
}

lt *make_input_port(FILE *stream) {
  lt *inf = make_object(LT_INPUT_PORT);
  input_port_stream(inf) = stream;
  input_port_linum(inf) = 1;
  input_port_colnum(inf) = 0;
  input_port_openp(inf) = TRUE;
  return inf;
}

lt *make_input_string_port(char *str) {
  FILE *stream = fmemopen(str, strlen(str), "r");
  return make_input_port(stream);
}

lt *make_mpflonum(mpf_t value) {
  lt *obj = make_object(LT_MPFLONUM);
  *mpflonum_value(obj) = *value;
  return obj;
}

lt *make_opcode(enum OPCODE_TYPE name, char *op, lt *oprands) {
  lt *obj = make_object(LT_OPCODE);
  opcode_name(obj) = name;
  opcode_op(obj) = op;
  opcode_oprands(obj) = oprands;
  return obj;
}

lt *make_output_port(FILE *stream) {
  lt *outf = make_object(LT_OUTPUT_PORT);
  output_port_stream(outf) = stream;
  output_port_linum(outf) = 1;
  output_port_colnum(outf) = 0;
  output_port_openp(outf) = TRUE;
  return outf;
}

lt *make_output_string_port(char *str) {
  FILE *stream = fmemopen(str, strlen(str), "w");
  return make_output_port(stream);
}

lt *make_package(lt *name, hash_table_t *symbol_table) {
  lt *obj = make_object(LT_PACKAGE);
  package_name(obj) = name;
  package_symbol_table(obj) = symbol_table;
  package_used_packages(obj) = the_empty_list;
  return obj;
}

lisp_object_t *make_pair(lisp_object_t *head, lisp_object_t *tail) {
  lisp_object_t *pair = make_object(LT_PAIR);
  pair_head(pair) = head;
  pair_tail(pair) = tail;
  return pair;
}

lisp_object_t *make_primitive(int arity, void *C_function, char *Lisp_name, int restp) {
  lisp_object_t *p = make_object(LT_PRIMITIVE);
  primitive_arity(p) = arity;
  primitive_func(p) = C_function;
  primitive_restp(p) = restp;
  primitive_Lisp_name(p) = Lisp_name;
  primitive_signature(p) = make_empty_list();
  return p;
}

lt *make_retaddr(lt *code, lt *env, lt *fn, int pc, int throw_flag, int sp) {
  lt *retaddr = make_object(LT_RETADDR);
  retaddr_code(retaddr) = code;
  retaddr_env(retaddr) = env;
  retaddr_fn(retaddr) = fn;
  retaddr_pc(retaddr) = pc;
  retaddr_throw_flag(retaddr) = throw_flag;
  return retaddr;
}

lt *make_string(int length, uint32_t *value) {
  lt *string = make_object(LT_STRING);
  string_length(string) = length;
  string_value(string) = value;
  return string;
}

lt *make_structure(lt *name, int nfield) {
  int pair_length(lt *);
  lt *obj = make_object(LT_STRUCT);
  structure_name(obj) = name;
  structure_data(obj) = make_vector(nfield);
  for (int i = 0; i < vector_length(structure_data(obj)); i++)
    vector_value(structure_data(obj))[i] = make_undef();
  return obj;
}

lisp_object_t *make_symbol(char *name, lt *package) {
  lisp_object_t *symbol = make_object(LT_SYMBOL);
  symbol_name(symbol) = name;
  symbol_macro(symbol) = the_undef;
  symbol_package(symbol) = package;
  symbol_value(symbol) = the_undef;
  return symbol;
}

lt *make_time(struct tm *value) {
  lt *obj = make_object(LT_TIME);
  time_value(obj) = value;
  return obj;
}

lt *make_type(enum TYPE type, char *name) {
  lt *t = make_object(LT_TYPE);
  type_tag(t) = type;
  type_name(t) = name;
  return t;
}

lt *make_unicode(char *data) {
  lt *obj = make_object(LT_UNICODE);
  unicode_data(obj) = data;
  return obj;
}

lisp_object_t *make_vector(int length) {
  lisp_object_t *vector = make_object(LT_VECTOR);
  vector_last(vector) = -1;
  vector_length(vector) = length;
  vector_value(vector) = GC_MALLOC(length * sizeof(lisp_object_t *));
  return vector;
}

/* Opcode */
lt *opcode_ref(enum OPCODE_TYPE opcode) {
  return &lt_codes[opcode];
}

void init_opcode_length(void) {
  int max = 0;
  for (int i = 0; i < sizeof(lt_codes) / sizeof(*lt_codes); i++) {
    lt *opcode = &lt_codes[i];
    if (strlen(opcode_op(opcode)) > max)
      max = strlen(opcode_op(opcode));
  }
  opcode_max_length = max;
}

/* Type */
lt *type_ref(enum TYPE type) {
  return &lt_types[type];
}

/* Unicode */
lt *make_unicode_char(char c) {
  return make_unicode(&tbl[(int)c]);
}

void init_character(void) {
  for (int i = 0; i < 256; i++)
    tbl[i] = i;
}
