/*
 * type.h
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <gmp.h>

#include "hash_table.h"

typedef struct lisp_object_t lisp_object_t;
typedef lisp_object_t lt;
typedef lt *(*f0)(void);
typedef lt *(*f1)(lt *);
typedef lt *(*f2)(lt *, lt *);
typedef lt *(*f3)(lt *, lt *, lt *);
typedef struct string_builder_t string_builder_t;

enum {
  CLOSE_ORIGIN,
  EOF_ORIGIN,
  FALSE_ORIGIN,
  NULL_ORIGIN,
  TRUE_ORIGIN,
  UNDEF_ORIGIN,
};

enum TYPE {
  /* tagged-pointer */
  LT_BOOL,
  LT_CHARACTER,
  LT_EMPTY_LIST,
  LT_FIXNUM,
  LT_TCLOSE,
  LT_TEOF,
  LT_TUNDEF,
  /* tagged-union */
  LT_BIGNUM,
  LT_ENVIRONMENT,
  LT_EXCEPTION,
  LT_FUNCTION,
  LT_FLOAT,
  LT_INPUT_PORT,
  LT_MPFLONUM,
  LT_OPCODE,
  LT_OUTPUT_PORT,
  LT_PACKAGE,
  LT_PAIR,
  LT_PRIMITIVE,
  LT_RETADDR,
  LT_STRING,
  LT_STRUCT,
  LT_SYMBOL,
  LT_TIME,
  LT_TYPE,
  LT_UNICODE,
  LT_VECTOR,
};

enum OPCODE_TYPE {
  CALL,
  CATCH,
  CHECKEX,
  CHKARITY,
  CHKTYPE,
  CONST,
  EXTENV,
  FN,
  GSET,
  GVAR,
  FJUMP,
  JUMP,
  LSET,
  LVAR,
  MOVEARGS,
  POP,
  POPENV,
  PRIM,
  RESTARGS,
  RETURN,
//  Primitive Function Instructions
  ADDI,
  CONS,
  DIVI,
  MULI,
  SUBI,
};

struct lisp_object_t {
  enum TYPE type;
  union {
    struct {
      mpz_t value;
    } bignum;
    struct {
      lt *bindings;
      lt *next;
    } environment;
    struct {
      int signal_flag;
      char *message;
      lt *backtrace;
      lt *exception_tag;
    } exception;
    struct {
      float value;
    } float_num;
    struct {
      lt *code;
//      There are two kinds of environments:
//      1. An environment records the information at compile time, only can be initialized when compiling;
//      2. An environment holds the parameters and local variables, only can be initialized when running.
//      `cenv' means `compile environment' and `renv' means `runtime environment'.
      lt *cenv, *renv;
      lt *args;
      lt *name;
    } function;
    struct {
      mpf_t value;
    } mpflonum;
    struct {
      enum OPCODE_TYPE name;
      char *op;
      lt *oprands;
    } opcode;
    struct {
      lt *name;
      hash_table_t *symbol_table;
      lt *used_packages;
    } package;
    struct {
      lt *head;
      lt *tail;
    } pair;
    struct {
      int colnum, linum, openp;
      FILE *stream;
    } port;
    struct {
      int arity;
      int restp;
      char *Lisp_name;
      void *C_function;
      lt *signature;
    } primitive;
    struct {
//      pc: The index of instructions executed before entering the instructions of callee
//      throw_flag: Indicates whether the current callee should throws the exception or not
//      code: The bytecode instructions of the last caller
//      env: The environment of the last caller
//      fn: The current callee. Use for two purposes
//          1. Use for constructing function calling chain when throwing exception
//          2. Stores the function to be used when modify its code pointer in the `yield' form
      int pc, throw_flag;
      lt *code;
      lt *env;
      lt *fn;
    } retaddr;
    struct {
      int length;
      char *value;
    } string;
    struct {
      lt *name;
      lt *data;
    } structure;
    struct {
      char *name;
      lt *global_value;
      lt *macro;
      lt *package;
    } symbol;
    struct {
      struct tm *value;
    } time;
    struct {
      enum TYPE tag;
      char *name;
    } type;
    struct {
      char *data;
    } unicode;
    struct {
      int last, length;
      lt **value;
    } vector;
  } u;
};

struct string_builder_t {
  int index, length;
  char *string;
};

#define FALSE 0
#define TRUE 1

#define character_value(x) (((intptr_t)x) >> CHAR_BITS)
#define fixnum_value(x) (((intptr_t)(x)) >> FIXNUM_BITS)

/* Accessor macros */
#define bignum_value(x) ((x)->u.bignum.value)
#define environment_bindings(x) ((x)->u.environment.bindings)
#define environment_next(x) ((x)->u.environment.next)
#define exception_msg(x) ((x)->u.exception.message)
#define exception_flag(x) ((x)->u.exception.signal_flag)
#define exception_backtrace(x) ((x)->u.exception.backtrace)
#define exception_tag(x) ((x)->u.exception.exception_tag)
#define float_value(x) ((x)->u.float_num.value)
#define function_args(x) ((x)->u.function.args)
#define function_cenv(x) ((x)->u.function.cenv)
#define function_renv(x) ((x)->u.function.renv)
#define function_code(x) ((x)->u.function.code)
#define function_name(x) ((x)->u.function.name)
#define input_port_colnum(x) ((x)->u.port.colnum)
#define input_port_stream(x) ((x)->u.port.stream)
#define input_port_linum(x) ((x)->u.port.linum)
#define input_port_openp(x) ((x)->u.port.openp)
#define mpflonum_value(x) ((x)->u.mpflonum.value)
#define opcode_name(x) ((x)->u.opcode.name)
#define opcode_op(x) ((x)->u.opcode.op)
#define opcode_oprands(x) ((x)->u.opcode.oprands)
#define output_port_colnum(x) ((x)->u.port.colnum)
#define output_port_stream(x) ((x)->u.port.stream)
#define output_port_linum(x) ((x)->u.port.linum)
#define output_port_openp(x) ((x)->u.port.openp)
#define package_name(x) ((x)->u.package.name)
#define package_symbol_table(x) ((x)->u.package.symbol_table)
#define package_used_packages(x) ((x)->u.package.used_packages)
#define pair_head(x) (x->u.pair.head)
#define pair_tail(x) (x->u.pair.tail)
#define primitive_Lisp_name(x) ((x)->u.primitive.Lisp_name)
#define primitive_arity(x) ((x)->u.primitive.arity)
#define primitive_func(x) ((x)->u.primitive.C_function)
#define primitive_signature(x) ((x)->u.primitive.signature)
#define primitive_restp(x) ((x)->u.primitive.restp)
#define retaddr_code(x) ((x)->u.retaddr.code)
#define retaddr_env(x) ((x)->u.retaddr.env)
#define retaddr_fn(x) ((x)->u.retaddr.fn)
#define retaddr_pc(x) ((x)->u.retaddr.pc)
#define retaddr_throw_flag(x) ((x)->u.retaddr.throw_flag)
#define string_length(x) ((x)->u.string.length)
#define string_value(x) ((x)->u.string.value)
#define structure_name(x) ((x)->u.structure.name)
#define structure_data(x) ((x)->u.structure.data)
#define symbol_name(x) ((x)->u.symbol.name)
#define symbol_macro(x) ((x)->u.symbol.macro)
#define symbol_package(x) ((x)->u.symbol.package)
#define symbol_value(x) ((x)->u.symbol.global_value)
#define time_value(x) ((x)->u.time.value)
#define type_tag(x) ((x)->u.type.tag)
#define type_name(x) ((x)->u.type.name)
#define unicode_data(x) ((x)->u.unicode.data)
#define vector_last(x) ((x)->u.vector.last)
#define vector_length(x) (x->u.vector.length)
#define vector_value(x) (x->u.vector.value)

/* Opcode accessor macros */
#define opcode_type(x) opcode_name(x)
#define opargn(x, n) (vector_value(opcode_oprands(x))[n])
#define oparg1(x) opargn(x, 0)
#define oparg2(x) opargn(x, 1)
#define oparg3(x) opargn(x, 2)
#define op_call_arity(x) oparg1(x)
#define op_chkarity_arity(x) oparg1(x)
#define op_chktype_pos(x) oparg1(x)
#define op_chktype_type(x) oparg2(x)
#define op_chktype_nargs(x) oparg3(x)
#define op_const_value(x) oparg1(x)
#define op_extenv_count(x) oparg1(x)
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
#define op_moveargs_count(x) oparg1(x)
#define op_prim_nargs(x) oparg1(x)
// The number of required parameters.
#define op_restargs_count(x) oparg1(x)

#endif /* TYPE_H_ */
