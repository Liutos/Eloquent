/*
 * object.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gc/gc.h>

#include "object.h"
#include "type.h"

int debug;
int is_check_exception;
int is_check_type;
lt *gensym_counter;
lt *null_env;
lt *package;
lt *pkgs;
lt *standard_error;
lt *standard_in;
lt *standard_out;
lt *symbol_list;
lt *the_argv;
lt *the_dot_symbol;
lt *the_empty_list;
lt *the_eof;
lt *the_false;
lt *the_true;
lt *the_undef;

#define DEFTYPE(tag, name) {.type=TYPE, .u={.type={tag, name}}}

struct lisp_object_t lt_types[VECTOR + 1] = {
    DEFTYPE(BOOL, "bool"),
    DEFTYPE(CHARACTER, "character"),
    DEFTYPE(EMPTY_LIST, "empty-list"),
    DEFTYPE(FIXNUM, "fixnum"),
    DEFTYPE(TCLOSE, "tclose"),
    DEFTYPE(TEOF, "teof"),
    DEFTYPE(TUNDEF, "tundef"),
    DEFTYPE(ENVIRONMENT, "environment"),
    DEFTYPE(EXCEPTION, "exception"),
    DEFTYPE(FUNCTION, "function"),
    DEFTYPE(FLOAT, "float"),
    DEFTYPE(INPUT_FILE, "input-file"),
    DEFTYPE(OPCODE, "opcode"),
    DEFTYPE(OUTPUT_FILE, "output-file"),
    DEFTYPE(PAIR, "pair"),
    DEFTYPE(PRIMITIVE_FUNCTION, "primitive-function"),
    DEFTYPE(RETADDR, "retaddr"),
    DEFTYPE(STRING, "string"),
    DEFTYPE(SYMBOL, "symbol"),
    DEFTYPE(TYPE, "type"),
    DEFTYPE(VECTOR, "vector"),
};

#define DEFCODE(name, op) {.type=OPCODE, .u={.opcode={name, op}}}

struct lisp_object_t lt_codes[CONS + 1] = {
    DEFCODE(ARGS, "ARGS"),
    DEFCODE(ARGSD, "ARGSD"),
    DEFCODE(CALL, "CALL"),
    DEFCODE(CATCH, "CATCH"),
    DEFCODE(CHECKEX, "CHECKEX"),
    DEFCODE(CHKTYPE, "CHKTYPE"),
    DEFCODE(CONST, "CONST"),
    DEFCODE(FN, "FN"),
    DEFCODE(GSET, "GSET"),
    DEFCODE(GVAR, "GVAR"),
    DEFCODE(FJUMP, "FJUMP"),
    DEFCODE(JUMP, "JUMP"),
    DEFCODE(LSET, "LSET"),
    DEFCODE(LVAR, "LVAR"),
    DEFCODE(POP, "POP"),
    DEFCODE(PRIM, "PRIM"),
    DEFCODE(RETURN, "RETURN"),
    DEFCODE(NO, "NO"),
    DEFCODE(CONS, "CONS"),
};

/* Type predicate */
int ischar(lt *object) {
  return ((intptr_t)object & CHAR_MASK) == CHAR_TAG;
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

mktype_pred(isenvironment, ENVIRONMENT)
mktype_pred(isexception, EXCEPTION)
mktype_pred(isfloat, FLOAT)
mktype_pred(isfunction, FUNCTION)
mktype_pred(isinput_file, INPUT_FILE)
mktype_pred(isoutput_file, OUTPUT_FILE)
mktype_pred(isopcode, OPCODE)
mktype_pred(ispair, PAIR)
mktype_pred(isprimitive, PRIMITIVE_FUNCTION)
mktype_pred(isstring, STRING)
mktype_pred(issymbol, SYMBOL)
mktype_pred(istype, TYPE)
mktype_pred(isvector, VECTOR)

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
  return isexception(object) && exception_flag(object) == TRUE;
}

int isdot(lisp_object_t *object) {
  return object == the_dot_symbol;
}

int isnull_env(lt *obj) {
  return obj == null_env;
}

int isnumber(lisp_object_t *object) {
  return isfixnum(object) || isfloat(object);
}

int isopcode_fn(lt *prim) {
  return isprimitive(prim) && primitive_opcode(prim) != NO;
}

int type_of(lisp_object_t *x) {
  if (isboolean(x))
    return BOOL;
  if (ischar(x))
    return CHARACTER;
  if (isnull(x))
    return EMPTY_LIST;
  if (isfixnum(x))
    return FIXNUM;
  if (isclose(x))
    return TCLOSE;
  if (iseof(x))
    return TEOF;
  if (isundef(x))
    return TUNDEF;
  assert(is_pointer(x));
  return x->type;
}

/* Hash Table */
ht_slot_t *make_slot(void *key, void *value, ht_slot_t *next) {
  ht_slot_t *sl = GC_MALLOC(sizeof(*sl));
  sl_key(sl) = key;
  sl_value(sl) = value;
  sl_next(sl) = next;
  return sl;
}

hash_table_t *make_hash_table(int length, hash_fn_t hash_fn, comp_fn_t comp_fn) {
  hash_table_t *ht = GC_MALLOC(sizeof(hash_table_t));
  ht_slots(ht) = GC_MALLOC(length * sizeof(ht_slot_t *));
  ht_length(ht) = length;
  ht_hash_fn(ht) = hash_fn;
  ht_comp_fn(ht) = comp_fn;
  return ht;
}

unsigned int compute_index(void *key, hash_table_t *ht) {
  hash_fn_t fn = ht_hash_fn(ht);
  int length = ht_length(ht);
  return (*fn)(key) % length;
}

int compare_in_ht(void *k1, void *k2, hash_table_t *ht) {
  comp_fn_t fn = ht_comp_fn(ht);
  return (*fn)(k1, k2);
}

ht_slot_t *raw_search_ht(void *key, hash_table_t *ht) {
  int index = compute_index(key, ht);
  ht_slot_t *sl = ht_slots(ht)[index];
  while (sl != NULL) {
    void *sk = sl_key(sl);
    if (compare_in_ht(sk, key, ht) == 0)
      return sl;
    sl = sl_next(sl);
  }
  return NULL;
}

void *search_ht(void *key, hash_table_t *ht) {
  ht_slot_t *sl = raw_search_ht(key, ht);
  if (sl != NULL)
    return sl_value(sl);
  else
    return NULL;
}

void set_ht(void *key, void *value, hash_table_t *ht) {
  ht_slot_t *sl = raw_search_ht(key, ht);
  if (sl != NULL)
    sl_value(sl) = value;
  else {
    unsigned int index = compute_index(key, ht);
    ht_slot_t *org = ht_slots(ht)[index];
    ht_slot_t *sl = make_slot(key, value, org);
    ht_slots(ht)[index] = sl;
  }
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

lisp_object_t *make_character(char value) {
  return (lt *)((((intptr_t)value) << CHAR_BITS) | CHAR_TAG);
}

lt *make_fixnum(int value) {
  return (lt *)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lt *make_environment(lt *bindings, lt *next) {
  lt *env = make_object(ENVIRONMENT);
  environment_bindings(env) = bindings;
  environment_next(env) = next;
  return env;
}

lt *make_exception(char *message, int signal_flag, lt *tag, lt *backtrace) {
  lt *ex = make_object(EXCEPTION);
  exception_msg(ex) = message;
  exception_flag(ex) = signal_flag;
  exception_backtrace(ex) = backtrace;
  exception_tag(ex) = tag;
  return ex;
}

lisp_object_t *make_float(float value) {
  lisp_object_t *flt_num = make_object(FLOAT);
  float_value(flt_num) = value;
  return flt_num;
}

lt *make_function(lt *cenv, lt *args, lt *code, lt *renv) {
  lt *func = make_object(FUNCTION);
  function_cenv(func) = cenv;
  function_args(func) = args;
  function_code(func) = code;
  function_name(func) = the_undef;
  function_renv(func) = renv;
  return func;
}

lisp_object_t *make_input_file(FILE *file) {
  lisp_object_t *inf = make_object(INPUT_FILE);
  input_file_file(inf) = file;
  input_file_linum(inf) = 1;
  input_file_colnum(inf) = 0;
  input_file_openp(inf) = TRUE;
  return inf;
}

lisp_object_t *make_output_file(FILE *file) {
  lisp_object_t *outf = make_object(OUTPUT_FILE);
  output_file_file(outf) = file;
  output_file_linum(outf) = 1;
  output_file_colnum(outf) = 0;
  output_file_openp(outf) = TRUE;
  return outf;
}

lt *make_package(lt *name, hash_table_t *symbol_table) {
  lt *obj = make_object(PACKAGE);
  package_name(obj) = name;
  package_symbol_table(obj) = symbol_table;
  package_used_packages(obj) = the_empty_list;
  return obj;
}

lisp_object_t *make_pair(lisp_object_t *head, lisp_object_t *tail) {
  lisp_object_t *pair = make_object(PAIR);
  pair_head(pair) = head;
  pair_tail(pair) = tail;
  return pair;
}

lisp_object_t *make_primitive(int arity, void *C_function, char *Lisp_name, int restp) {
  lisp_object_t *p = make_object(PRIMITIVE_FUNCTION);
  primitive_arity(p) = arity;
  primitive_func(p) = C_function;
  primitive_restp(p) = restp;
  primitive_Lisp_name(p) = Lisp_name;
  primitive_signature(p) = make_empty_list();
  primitive_opcode(p) = NO;
  return p;
}

lt *make_retaddr(lt *code, lt *env, lt *fn, int pc, int throw_flag, int sp) {
  lt *retaddr = make_object(RETADDR);
  retaddr_code(retaddr) = code;
  retaddr_env(retaddr) = env;
  retaddr_fn(retaddr) = fn;
  retaddr_pc(retaddr) = pc;
  retaddr_throw_flag(retaddr) = throw_flag;
  retaddr_sp(retaddr) = sp;
  return retaddr;
}

string_builder_t *make_str_builder(void) {
  string_builder_t *sb = GC_MALLOC(sizeof(*sb));
  sb->length = 20;
  sb->string = GC_MALLOC(sb->length * sizeof(char));
  sb->index = 0;
  return sb;
}

lisp_object_t *make_string(char *value) {
  lisp_object_t *string = make_object(STRING);
  string_value(string) = value;
  return string;
}

lisp_object_t *make_symbol(char *name, lt *package) {
  lisp_object_t *symbol = make_object(SYMBOL);
  symbol_name(symbol) = name;
  symbol_macro(symbol) = the_undef;
  symbol_package(symbol) = package;
  symbol_value(symbol) = the_undef;
  return symbol;
}

lt *make_type(enum TYPE type, char *name) {
  lt *t = make_object(TYPE);
  type_tag(t) = type;
  type_name(t) = name;
  return t;
}

lisp_object_t *make_vector(int length) {
  lisp_object_t *vector = make_object(VECTOR);
  vector_last(vector) = -1;
  vector_length(vector) = length;
  vector_value(vector) = GC_MALLOC(length * sizeof(lisp_object_t *));
  return vector;
}

/* Opcode constructor functions */
lt *make_opcode(enum OPCODE_TYPE name, char *op, lt *oprands) {
  lt *obj = make_object(OPCODE);
  opcode_name(obj) = name;
  opcode_op(obj) = op;
  opcode_oprands(obj) = oprands;
  return obj;
}

lt *mkopcode(enum OPCODE_TYPE name, char *op, int arity, ...) {
  lt *oprands = make_vector(arity);
  va_list ap;
  va_start(ap, arity);
  for (int i = 0; i < arity; i++)
    vector_value(oprands)[i] = va_arg(ap, lt *);
  vector_last(oprands) = arity - 1;
  return make_opcode(name, op, oprands);
}

lt *make_op_argsd(lt *length) {
  return mkopcode(ARGSD, "ARGSD", 1, length);
}

lisp_object_t *make_op_args(lisp_object_t *length) {
  return mkopcode(ARGS, "ARGS", 1, length);
}

lisp_object_t *make_op_call(lisp_object_t *arity) {
  return mkopcode(CALL, "CALL", 1, arity);
}

lt *make_op_checkex(void) {
  return mkopcode(CHECKEX, "CHECKEX", 0);
}

lt *make_op_chktype(lt *position, lt *target_type, lt *nargs) {
  return mkopcode(CHKTYPE, "CHKTYPE", 3, position, target_type, nargs);
}

lisp_object_t *make_op_const(lisp_object_t *value) {
  return mkopcode(CONST, "CONST", 1, value);
}

lisp_object_t *make_op_fjump(lisp_object_t *label) {
  return mkopcode(FJUMP, "FJUMP", 1, label);
}

lisp_object_t *make_op_fn(lisp_object_t *func) {
  return mkopcode(FN, "FN", 1, func);
}

lisp_object_t *make_op_gset(lisp_object_t *symbol) {
  return mkopcode(GSET, "GSET", 1, symbol);
}

lisp_object_t *make_op_gvar(lisp_object_t *symbol) {
  return mkopcode(GVAR, "GVAR", 1, symbol);
}

lisp_object_t *make_op_jump(lisp_object_t *label) {
  return mkopcode(JUMP, "JUMP", 1, label);
}

lt *make_op_lset(lt *i, lt *j, lt *symbol) {
  return mkopcode(LSET, "LSET", 3, i, j, symbol);
}

lt *make_op_lvar(lt *i, lt *j, lt *symbol) {
  return mkopcode(LVAR, "LVAR", 3, i, j, symbol);
}

lisp_object_t *make_op_pop(void) {
  return mkopcode(POP, "POP", 0);
}

lisp_object_t *make_op_prim(lisp_object_t *nargs) {
  return mkopcode(PRIM, "PRIM", 1, nargs);
}

lisp_object_t *make_op_return() {
  return mkopcode(RETURN, "RETURN", 0);
}

lt *make_op_catch(void) {
  return mkopcode(CATCH, "CATCH", 0);
}

lt *make_fn_inst(lt *prim) {
  return make_pair(mkopcode(primitive_opcode(prim), "", 0), the_empty_list);
}

// Package
lt *lt_package_name(lt *pkg) {
  return package_name(pkg);
}

lt *search_package(char *name, lt *packages) {
  while (ispair(packages)) {
    lt *pkg = pair_head(packages);
    if (strcmp(string_value(package_name(pkg)), name) == 0)
      return pkg;
    packages = pair_tail(packages);
  }
  return NULL;
}

lt *ensure_package(char *name) {
  lt *result = search_package(name, pkgs);
  if (result)
    return result;
  lt *pkg = make_package(make_string(name), make_symbol_table());
  pkgs = make_pair(pkg, pkgs);
  symbol_value(find_or_create_symbol("*package*", pkg)) = pkg;
  return pkg;
}

void use_package_in(lt *used, lt *pkg) {
  package_used_packages(pkg) =
      make_pair(used, package_used_packages(pkg));
}

/* Symbol */
// The following algorithm comes from http://bbs.csdn.net/topics/350030230
unsigned int symbol_hash_fn(void *symbol) {
  char *name = (char *)symbol;
  int seed = 131;
  unsigned int hash = 0;
  while (*name != '\0') {
    hash = hash * seed + *name;
    name++;
  }
  return hash & 0x7FFFFFFF;
}

int symbol_comp_fn(void *s1, void *s2) {
  char *n1 = (char *)s1;
  char *n2 = (char *)s2;
  return strcmp(n1, n2);
}

hash_table_t *make_symbol_table(void) {
  return make_hash_table(31, symbol_hash_fn, symbol_comp_fn);
}

// Search the symbol with `name' in `symbol_table'
lt *search_symbol_table(char *name, hash_table_t *symbol_table) {
  return search_ht((void *)name, symbol_table);
}

lt *find_symbol(char *name, lt *package) {
  lt *sym = search_symbol_table(name, package_symbol_table(package));
  if (sym)
    return sym;
  lt *useds = package_used_packages(package);
  while (ispair(useds)) {
    lt *pkg = pair_head(useds);
    sym = search_symbol_table(name, package_symbol_table(pkg));
    if (sym)
      return sym;
    useds = pair_tail(useds);
  }
  return NULL;
}

lt *find_or_create_symbol(char *name, lt *package) {
  lt *result = find_symbol(name, package);
  if (result)
    return result;
  lt *sym = make_symbol(name, package);
  set_ht((void *)name, (void *)sym, package_symbol_table(package));
  return sym;
}

lt *lt_symbol_package(lt *symbol) {
  return symbol_package(symbol);
}

lt *lt_exception_tag(lt *exception) {
  return exception_tag(exception);
}

lt *lt_type_name(lt *type) {
  return S(type_name(type));
}

void init_packages(void) {
  pkgs = make_empty_list();
  lt *lisp = ensure_package("233");
//  (defpackage :233-user
//    (:use :233))
  lt *user = ensure_package("233-user");
  use_package_in(lisp, user);
// Set the current package
  package = user;
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
  standard_error = make_output_file(stderr);
  standard_in = make_input_file(stdin);
  standard_out = make_output_file(stdout);
  symbol_list = the_empty_list;
  the_undef = make_undef();

//  Packages initialization
  init_packages();

// Global variables initialization
  symbol_value(S("*ARGV*")) = the_argv;
  symbol_value(S("*gensym-counter*")) = gensym_counter;
  symbol_value(S("*standard-error*")) = standard_error;
  symbol_value(S("*standard-output*")) = standard_out;
  symbol_value(S("*standard-input*")) = standard_in;

  /* Symbol initialization */
  the_dot_symbol = S(".");
}
