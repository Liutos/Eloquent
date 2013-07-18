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

#define OBJECT_INIT_COUNT 1000

/* tagged-pointer types */
/* immediate objects */
/* PART: object.c */
/* Global variables */
lisp_object_t *dot_symbol;
lisp_object_t *false;
lisp_object_t *true;
lisp_object_t *null_env;
lisp_object_t *null_list;
lt *object_pool;                        /* An array contains lt. */
lisp_object_t *standard_in;
lisp_object_t *standard_out;
lisp_object_t *symbol_list;
lisp_object_t *undef_object;
lt *free_objects;

/* Type predicate */
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
mktype_pred(ispair, PAIR)
mktype_pred(isprimitive, PRIMITIVE_FUNCTION)
mktype_pred(isstring, STRING)
mktype_pred(issymbol, SYMBOL)
mktype_pred(isvector, VECTOR)

int ischar(lt *object) {
  return ((int)object & CHAR_MASK) == CHAR_TAG;
}

int isfixnum(lt *object) {
  return ((int)object & FIXNUM_MASK) == FIXNUM_TAG;
}

int isdot(lisp_object_t *object) {
  return object == dot_symbol;
}

int is_immediate(lt *object) {
  return ((int)object & IMMEDIATE_MASK) == IMMEDIATE_TAG;
}

int is_tag_immediate(lt *object, int origin) {
  return is_immediate(object) && ((int)object >> IMMEDIATE_BITS) == origin;
}

#define mkim_pred(func_name, origin)		\
  int func_name(lt *object) {			\
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

int isnumber(lisp_object_t *object) {
  return isfixnum(object) || isfloat(object);
}

int type_of(lisp_object_t *x) {
  if (isfixnum(x))
    return FIXNUM;
  if (ischar(x))
    return CHARACTER;
  if (isnull(x))
    return EMPTY_LIST;
  if (isboolean(x))
    return BOOL;
  if (iseof(x))
    return TEOF;
  if (isundef(x))
    return UNDEF;
  if (isclose(x))
    return TCLOSE;
  assert(is_pointer(x));
  return x->type;
}

/* Initialization */
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

/* DONE: Collects this part to file object.c */
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

/* Constructors */
lt *allocate_object(void) {
	if (free_objects == NULL)
		trigger_gc();
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

lisp_object_t *make_function(lisp_object_t *env, lisp_object_t *args, lisp_object_t *code) {
  lisp_object_t *func = make_object(FUNCTION);
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
  return inf;
}

lisp_object_t *make_output_file(FILE *file) {
  lisp_object_t *outf = make_object(OUTPUT_FILE);
  output_file_file(outf) = file;
  output_file_linum(outf) = 1;
  output_file_colnum(outf) = 0;
  return outf;
}

lisp_object_t *make_pair(lisp_object_t *head, lisp_object_t *tail) {
  lisp_object_t *pair = make_object(PAIR);
  pair_head(pair) = head;
  pair_tail(pair) = tail;
  return pair;
}

lisp_object_t *make_primitive(int arity, void *C_function, char *Lisp_name) {
  lisp_object_t *p = make_object(PRIMITIVE_FUNCTION);
  primitive_arity(p) = arity;
  primitive_func(p) = C_function;
  primitive_Lisp_name(p) = Lisp_name;
  return p;
}

lisp_object_t *make_retaddr(lisp_object_t *code, lisp_object_t *env, int pc) {
  lisp_object_t *retaddr = make_object(RETADDR);
  retaddr_code(retaddr) = code;
  retaddr_env(retaddr) = env;
  retaddr_pc(retaddr) = pc;
  return retaddr;
}

lisp_object_t *make_string(char *value) {
  lisp_object_t *string = make_object(STRING);
  string->u.string.value = value;
  return string;
}

lisp_object_t *make_symbol(char *name) {
  lisp_object_t *symbol = make_object(SYMBOL);
  symbol->u.symbol.name = name;
  symbol_value(symbol) = undef_object;
  return symbol;
}

lisp_object_t *make_vector(int length) {
  lisp_object_t *vector = make_object(VECTOR);
  vector_last(vector) = -1;
  vector_length(vector) = length;
  vector_value(vector) = checked_malloc(length * sizeof(lisp_object_t *));
  return vector;
}

/* Opcode constructors */
lt *make_opcode(enum OPCODE_TYPE name, lt *oprands) {
  lt *obj = make_object(OPCODE);
  opcode_name(obj) = name;
  opcode_oprands(obj) = oprands;
  return obj;
}

lt *mkopcode(enum OPCODE_TYPE name, int arity, ...) {
  lt *oprands = make_vector(arity);
  va_list ap;
  va_start(ap, arity);
  for (int i = 0; i < arity; i++)
    vector_value(oprands)[i] = va_arg(ap, lt *);
  vector_last(oprands) = arity - 1;
  return make_opcode(name, oprands);
}

lisp_object_t *make_op_args(lisp_object_t *length) {
  return mkopcode(ARGS, 1, length);
}

lisp_object_t *make_op_call(lisp_object_t *arity) {
  return mkopcode(CALL, 1, arity);
}

lisp_object_t *make_op_const(lisp_object_t *value) {
  return mkopcode(CONST, 1, value);
}

lisp_object_t *make_op_fjump(lisp_object_t *label) {
  return mkopcode(FJUMP, 1, label);
}

lisp_object_t *make_op_fn(lisp_object_t *func) {
  return mkopcode(FN, 1, func);
}

lisp_object_t *make_op_gset(lisp_object_t *symbol) {
  return mkopcode(GSET, 1, symbol);
}

lisp_object_t *make_op_gvar(lisp_object_t *symbol) {
  return mkopcode(GVAR, 1, symbol);
}

lisp_object_t *make_op_jump(lisp_object_t *label) {
  return mkopcode(JUMP, 1, label);
}

lisp_object_t *make_op_lset(lisp_object_t *i, lisp_object_t *j, lisp_object_t *symbol) {
  return mkopcode(LSET, 3, i, j, symbol);
}

lisp_object_t *make_op_lvar(lisp_object_t *i, lisp_object_t *j, lisp_object_t *symbol) {
  return mkopcode(LVAR, 3, i, j, symbol);
}

lisp_object_t *make_op_pop(void) {
  return mkopcode(POP, 0);
}

lisp_object_t *make_op_prim(lisp_object_t *nargs) {
  return mkopcode(PRIM, 1, nargs);
}

lisp_object_t *make_op_return(void) {
  return mkopcode(RETURN, 0);
}

lt *make_op_catch(lt *type_name, lt *handler) {
  return mkopcode(CATCH, 2, type_name, handler);
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
  false = make_false();
  true = make_true();
  null_list = make_empty_list();
  null_env = null_list;
  standard_in = make_input_file(stdin);
  standard_out = make_output_file(stdout);
  symbol_list = null_list;
  undef_object = make_undef();
  /* Symbol initialization */
  dot_symbol = S(".");

  symbol_value(S("*standard-output*")) = standard_out;
  symbol_value(S("*standard-input*")) = standard_in;
}
