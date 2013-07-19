/*
 * type.h
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <stdio.h>

typedef struct lisp_object_t lisp_object_t;
typedef lisp_object_t lt;
typedef lt *(*f0)(void);
typedef lisp_object_t *(*f1)(lisp_object_t *);
typedef lisp_object_t *(*f2)(lisp_object_t *, lisp_object_t *);
typedef lisp_object_t *(*f3)(lisp_object_t *, lisp_object_t *, lisp_object_t *);

/* StringBuilder */
typedef struct string_builder_t {
  char *string;
  int length;
  int index;
} string_builder_t;

enum ORIGINS {
  EOF_ORIGIN,
  FALSE_ORIGIN,
  NULL_ORIGIN,
  TRUE_ORIGIN,
  UNDEF_ORIGIN,
  CLOSE_ORIGIN,
};

/* TODO: The support for Unicode. */
/* TODO: Implements the arbitrary precision arithmetic numeric types. */
enum TYPE {
  /* tagged-pointer */
  BOOL,
  CHARACTER,
  EMPTY_LIST,
  FIXNUM,
  TCLOSE,
  TEOF,
  UNDEF,
  /* tagged-union */
  FUNCTION,
  EXCEPTION,
  FLOAT,
  INPUT_FILE,
  MACRO,
  OPCODE,
  OUTPUT_FILE,
  PAIR,
  PRIMITIVE_FUNCTION,
  RETADDR,
  STRING,
  SYMBOL,
  VECTOR,
};

enum OPCODE_TYPE {
  ARGS,
  CALL,
  CATCH,
  CONST,
  FN,
  GSET,
  GVAR,
  FJUMP,
  JUMP,
  LSET,
  LVAR,
  POP,
  PRIM,
  RETURN,
};

struct lisp_object_t {
  int gc_mark_flag;
  int use_flag;
  lt *next;
  enum TYPE type;
  union {
    struct {
      char *message;
      int signal_flag;
    } exception;
    struct {
      float value;
    } float_num;
    struct {
      lisp_object_t *code;
      lisp_object_t *env;
      lisp_object_t *args;
    } function;
    struct {
      FILE *file;
      int linum;
      int colnum;
    } input_file;
    struct {
      lt *procedure;
      lt *environment;
    } macro;
    struct {
      enum OPCODE_TYPE name;
      lt *oprands;
    } opcode;
    struct {
      FILE *file;
      int linum;
      int colnum;
    } output_file;
    struct {
      lisp_object_t *head;
      lisp_object_t *tail;
    } pair;
    struct {
      int arity;
      void *C_function;
      char *Lisp_name;
    } primitive;
    struct {
      lisp_object_t *code;
      lisp_object_t *env;
      int pc;
      int throw_flag;
    } retaddr;
    struct {
      char *value;
      int length;
    } string;
    struct {
      char *name;
      lisp_object_t *global_value;
    } symbol;
    struct {
      int last;
      int length;
      lisp_object_t **value;
    } vector;
  } u;
};

#define FALSE 0
#define TRUE 1
#define pub

/* Accessors */
#define character_value(x) (((int)x) >> CHAR_BITS)
#define exception_msg(x) ((x)->u.exception.message)
#define exception_flag(x) ((x)->u.exception.signal_flag)
#define fixnum_value(x) (((int)(x)) >> FIXNUM_BITS)
#define float_value(x) ((x)->u.float_num.value)
#define function_args(x) ((x)->u.function.args)
#define function_env(x) ((x)->u.function.env)
#define function_code(x) ((x)->u.function.code)
#define input_file_colnum(x) ((x)->u.input_file.colnum)
#define input_file_file(x) ((x)->u.input_file.file)
#define input_file_linum(x) ((x)->u.input_file.linum)
#define macro_procedure(x) ((x)->u.macro.procedure)
#define macro_environment(x) ((x)->u.macro.environment)
#define opcode_name(x) ((x)->u.opcode.name)
#define opcode_oprands(x) ((x)->u.opcode.oprands)
#define output_file_colnum(x) ((x)->u.output_file.colnum)
#define output_file_file(x) ((x)->u.output_file.file)
#define output_file_linum(x) ((x)->u.output_file.linum)
#define pair_head(x) (x->u.pair.head)
#define pair_tail(x) (x->u.pair.tail)
#define primitive_Lisp_name(x) ((x)->u.primitive.Lisp_name)
#define primitive_arity(x) ((x)->u.primitive.arity)
#define primitive_func(x) ((x)->u.primitive.C_function)
#define retaddr_code(x) ((x)->u.retaddr.code)
#define retaddr_env(x) ((x)->u.retaddr.env)
#define retaddr_pc(x) ((x)->u.retaddr.pc)
#define retaddr_throw_flag(x) ((x)->u.retaddr.throw_flag)
#define string_value(x) ((x)->u.string.value)
#define symbol_name(x) ((x)->u.symbol.name)
#define symbol_value(x) ((x)->u.symbol.global_value)
#define vector_last(x) ((x)->u.vector.last)
#define vector_length(x) (x->u.vector.length)
#define vector_value(x) (x->u.vector.value)
/* Opcode Accessors */
#define opcode_type(x) opcode_name(x)
#define opargn(x, n) (vector_value(opcode_oprands(x))[n])
#define oparg1(x) opargn(x, 0)
#define oparg2(x) opargn(x, 1)
#define oparg3(x) opargn(x, 2)

#define op_args_arity(x) oparg1(x)
#define op_call_arity(x) oparg1(x)
#define op_const_value(x) oparg1(x)
#define op_fjump_label(x) oparg1(x)
#define op_fn_func(x) oparg1(x)
#define op_gset_var(x) oparg1(x)
#define op_gvar_var(x) oparg1(x)
#define op_jump_label(x) oparg1(x)
#define op_lset_i(x) oparg1(x)
#define op_lset_j(x) oparg2(x)
#define op_lset_var(x) oparg3(x)
#define op_lvar_i(x) oparg1(x)
#define op_lvar_j(x) oparg2(x)
#define op_lvar_var(x) oparg3(x)
#define op_prim_nargs(x) oparg1(x)

#endif /* TYPE_H_ */
