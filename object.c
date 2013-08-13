/*
 * object.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "type.h"

lt *free_objects;
lt *gensym_counter;
lt *null_env;
lt *object_pool;
lt *standard_in;
lt *standard_out;
lt *symbol_list;
lt *the_dot_symbol;
lt *the_empty_list;
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
    DEFTYPE(EXCEPTION, "exception"),
    DEFTYPE(FUNCTION, "function"),
    DEFTYPE(FLOAT, "float"),
    DEFTYPE(INPUT_FILE, "input-file"),
    DEFTYPE(MACRO, "macro"),
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

#define OBJECT_INIT_COUNT 2000

/* Type predicate */
int ischar(lt *object) {
  return ((int)object & CHAR_MASK) == CHAR_TAG;
}

int isfixnum(lt *object) {
  return ((int)object & FIXNUM_MASK) == FIXNUM_TAG;
}

int is_pointer(lt *object) {
  return ((int)object & POINTER_MASK) == POINTER_TAG;
}

int is_of_type(lisp_object_t *object, enum TYPE type) {
  return is_pointer(object) && (object->type == type? TRUE: FALSE);
}

#define mktype_pred(func_name, type)            \
  int func_name(lisp_object_t *object) {        \
    return is_of_type(object, type);            \
  }

mktype_pred(isexception, EXCEPTION)
mktype_pred(isfloat, FLOAT)
mktype_pred(isfunction, FUNCTION)
mktype_pred(isinput_file, INPUT_FILE)
mktype_pred(ismacro, MACRO)
mktype_pred(isoutput_file, OUTPUT_FILE)
mktype_pred(isopcode, OPCODE)
mktype_pred(ispair, PAIR)
mktype_pred(isprimitive, PRIMITIVE_FUNCTION)
mktype_pred(isstring, STRING)
mktype_pred(issymbol, SYMBOL)
mktype_pred(istype, TYPE)
mktype_pred(isvector, VECTOR)

int is_immediate(lt *object) {
  return ((int)object & IMMEDIATE_MASK) == IMMEDIATE_TAG;
}

int is_tag_immediate(lt *object, int origin) {
  return is_immediate(object) && ((int)object >> IMMEDIATE_BITS) == origin;
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

int isnumber(lisp_object_t *object) {
  return isfixnum(object) || isfloat(object);
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

void init_object_pool(void) {
  static int flag = 0;
  if (flag == 1) return;
  flag = 1;
  object_pool = calloc(OBJECT_INIT_COUNT, sizeof(lt));
  for (int i = 0; i < OBJECT_INIT_COUNT - 1; i++)
    object_pool[i].next = &object_pool[i + 1];
  object_pool[OBJECT_INIT_COUNT - 1].next = NULL;
  free_objects = &object_pool[0];
}

void mark_lt_object(lt *object) {
	if (!is_pointer(object) || object == NULL)
		return;
	if (object->gc_mark_flag == TRUE)
		return;
	object->gc_mark_flag = TRUE;
	switch (type_of(object)) {
	case FUNCTION:
		mark_lt_object(function_args(object));
		mark_lt_object(function_code(object));
		mark_lt_object(function_env(object));
		break;
	case OPCODE:
		mark_lt_object(opcode_oprands(object));
		break;
	case PAIR:
		mark_lt_object(pair_head(object));
		mark_lt_object(pair_tail(object));
		break;
	case SYMBOL:
		mark_lt_object(symbol_value(object));
		break;
	case VECTOR:
		for (int i = 0; i <= vector_last(object); i++)
			mark_lt_object(vector_value(object)[i]);
		break;
	default :;
	}
}

void mark_all(void) {
	mark_lt_object(symbol_list);
}

void sweep_all(void) {
	for (int i = 0; i < OBJECT_INIT_COUNT; i++) {
		lt *obj = &object_pool[i];
		if (obj->use_flag == TRUE && obj->gc_mark_flag == FALSE) {
			obj->use_flag = FALSE;
			obj->next = free_objects;
			free_objects = obj;
		} else
			obj->gc_mark_flag = FALSE;
	}
}

void trigger_gc(void) {
	mark_all();
	sweep_all();
}

/* Constructor functions */
lt *allocate_object(void) {
  if (free_objects == NULL) {
    printf("Pool is full, program terminated.\n");
    exit(1);
  }
  lt *obj = free_objects;
  free_objects = free_objects->next;
  obj->gc_mark_flag = FALSE;
  obj->use_flag = TRUE;
  return obj;
}

void *checked_malloc(size_t size) {
  void *p = malloc(size);
  assert(p != NULL);
  return p;
}

lisp_object_t *make_object(enum TYPE type) {
  lt *obj = allocate_object();
  obj->type = type;
  return obj;
}

#define MAKE_IMMEDIATE(origin) \
  ((lt *)(((int)origin << IMMEDIATE_BITS) | IMMEDIATE_TAG))

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
  return (lt *)((((int)value) << CHAR_BITS) | CHAR_TAG);
}

lt *make_fixnum(int value) {
  return (lt *)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lt *make_exception(char *message, int signal_flag) {
  lt *ex = make_object(EXCEPTION);
  exception_msg(ex) = message;
  exception_flag(ex) = signal_flag;
  return ex;
}

lisp_object_t *make_float(float value) {
  lisp_object_t *flt_num = make_object(FLOAT);
  float_value(flt_num) = value;
  return flt_num;
}

lt *make_function(lt *env, lt *args, lt *code) {
  lt *func = make_object(FUNCTION);
  function_env(func) = env;
  function_args(func) = args;
  function_code(func) = code;
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

lt *make_macro(lt *procedure, lt *environment) {
  lt *obj = make_object(MACRO);
  macro_procedure(obj) = procedure;
  macro_environment(obj) = environment;
  return obj;
}

lisp_object_t *make_output_file(FILE *file) {
  lisp_object_t *outf = make_object(OUTPUT_FILE);
  output_file_file(outf) = file;
  output_file_linum(outf) = 1;
  output_file_colnum(outf) = 0;
  output_file_openp(outf) = TRUE;
  return outf;
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
  return p;
}

lt *make_retaddr(lt *code, lt *env, int pc, int throw_flag, int sp) {
  lt *retaddr = make_object(RETADDR);
  retaddr_code(retaddr) = code;
  retaddr_env(retaddr) = env;
  retaddr_pc(retaddr) = pc;
  retaddr_throw_flag(retaddr) = throw_flag;
  retaddr_sp(retaddr) = sp;
  return retaddr;
}

string_builder_t *make_str_builder(void) {
  string_builder_t *sb = malloc(sizeof(*sb));
  sb->length = 20;
  sb->string = malloc(sb->length * sizeof(char));
  sb->index = 0;
  return sb;
}

lisp_object_t *make_string(char *value) {
  lisp_object_t *string = make_object(STRING);
  string->u.string.value = value;
  return string;
}

lisp_object_t *make_symbol(char *name) {
  lisp_object_t *symbol = make_object(SYMBOL);
  symbol->u.symbol.name = name;
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
  vector_value(vector) = checked_malloc(length * sizeof(lisp_object_t *));
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

lt *make_op_decl(lt *symbol) {
  return mkopcode(DECL, "DECL", 1, symbol);
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

lt *make_op_macro(lt *func) {
	return mkopcode(MACROFN, "MACROFN", 1, func);
}

lisp_object_t *make_op_pop(void) {
  return mkopcode(POP, "POP", 0);
}

lisp_object_t *make_op_prim(lisp_object_t *nargs) {
  return mkopcode(PRIM, "PRIM", 1, nargs);
}

lisp_object_t *make_op_return(void) {
  return mkopcode(RETURN, "RETURN", 0);
}

lt *make_op_catch(void) {
  return mkopcode(CATCH, "CATCH", 0);
}

/* TODO: Use a hash table for storing symbols. */
lisp_object_t *find_or_create_symbol(char *name) {
  lisp_object_t *tmp_sym_list = symbol_list;
  while (ispair(tmp_sym_list)) {
    lisp_object_t *sym = pair_head(tmp_sym_list);
    char *key = symbol_name(sym);
    if (strcmp(key, name) == 0)
      return sym;
    tmp_sym_list = pair_tail(tmp_sym_list);
  }
  lisp_object_t *sym = make_symbol(name);
  symbol_list = make_pair(sym, symbol_list);
  return sym;
}

void init_global_variable(void) {
  init_object_pool();
  /* Initialize global variables */
  the_false = make_false();
  the_true = make_true();
  the_empty_list = make_empty_list();
  gensym_counter = make_fixnum(0);
  null_env = the_empty_list;
  standard_in = make_input_file(stdin);
  standard_out = make_output_file(stdout);
  symbol_list = the_empty_list;
  the_undef = make_undef();

  symbol_value(S("*gensym-counter*")) = gensym_counter;
  symbol_value(S("*standard-output*")) = standard_out;
  symbol_value(S("*standard-input*")) = standard_in;

  /* Symbol initialization */
  the_dot_symbol = S(".");
}
