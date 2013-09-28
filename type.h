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

typedef unsigned int (*hash_fn_t)(void *);
typedef int (*comp_fn_t)(void *, void *);
typedef struct ht_slot_t ht_slot_t;
typedef struct hash_table_t hash_table_t;
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
  TUNDEF,
  /* tagged-union */
  ENVIRONMENT,
  EXCEPTION,
  FUNCTION,
  FLOAT,
  INPUT_FILE,
  OPCODE,
  OUTPUT_FILE,
  PACKAGE,
  PAIR,
  PRIMITIVE_FUNCTION,
  RETADDR,
  STRING,
  SYMBOL,
  TYPE,
  VECTOR,
};

enum OPCODE_TYPE {
  ARGS,
  ARGSD,
  CALL,
  CATCH,
  CHECKEX,
  CHKTYPE,
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
//  Primitive Function Instructions
  CONS,
};

/* General Hash Table Definition */
struct ht_slot_t {
  void *key;
  void *value;
  ht_slot_t *next;
};

// slots: An array for storing key-values
// length: Length of slots
// hash_fn: Pointer to function for generating hash value used as index in slots
// comp_fn: Pointer to function for comparing two keys when their hash value is equal
struct hash_table_t {
  ht_slot_t **slots;
  int length;
  hash_fn_t hash_fn;
  comp_fn_t comp_fn;
};

struct lisp_object_t {
  enum TYPE type;
  union {
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
      int colnum, linum, openp;
      FILE *file;
    } input_file;
    struct {
      int colnum, index, linum;
      char *value;
    } input_string;
    struct {
      enum OPCODE_TYPE name;
      char *op;
      lt *oprands;
    } opcode;
    struct {
      int colnum, linum, openp;
      FILE *file;
    } output_file;
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
      int arity;
      int restp;
      char *Lisp_name;
      void *C_function;
      lt *signature;
    } primitive;
    struct {
//      pc: The index of instructions executed before entering the instructions of callee
//      sp: The index of the element on argument stack before entering the callee
//      throw_flag: Indicates whether the current callee should throws the exception or not
//      code: The bytecode instructions of the last caller
//      env: The environment of the last caller
//      fn: The current callee. Use for two purposes
//          1. Use for constructing function calling chain when throwing exception
//          2. Stores the function to be used when modify its code pointer in the `yield' form
      int pc, sp, throw_flag;
      lt *code;
      lt *env;
      lt *fn;
    } retaddr;
    struct {
      int length;
      char *value;
    } string;
    struct {
      char *name;
      lt *global_value;
      lt *macro;
      lt *package;
    } symbol;
    struct {
      enum TYPE tag;
      char *name;
    } type;
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

/* Hash Table */
#define sl_key(x) ((x)->key)
#define sl_value(x) ((x)->value)
#define sl_next(x) ((x)->next)
#define ht_slots(x) ((x)->slots)
#define ht_length(x) ((x)->length)
#define ht_hash_fn(x) ((x)->hash_fn)
#define ht_comp_fn(x) ((x)->comp_fn)

/* Accessor macros */
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
#define input_file_colnum(x) ((x)->u.input_file.colnum)
#define input_file_file(x) ((x)->u.input_file.file)
#define input_file_linum(x) ((x)->u.input_file.linum)
#define input_file_openp(x) ((x)->u.input_file.openp)
#define input_string_colnum(x) ((x)->u.input_string.colnum)
#define input_string_index(x) ((x)->u.input_string.index)
#define input_string_linum(x) ((x)->u.input_string.linum)
#define input_string_value(x) ((x)->u.input_string.value)
#define opcode_name(x) ((x)->u.opcode.name)
#define opcode_op(x) ((x)->u.opcode.op)
#define opcode_oprands(x) ((x)->u.opcode.oprands)
#define output_file_colnum(x) ((x)->u.output_file.colnum)
#define output_file_file(x) ((x)->u.output_file.file)
#define output_file_linum(x) ((x)->u.output_file.linum)
#define output_file_openp(x) ((x)->u.output_file.openp)
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
#define retaddr_sp(x) ((x)->u.retaddr.sp)
#define string_value(x) ((x)->u.string.value)
#define symbol_name(x) ((x)->u.symbol.name)
#define symbol_macro(x) ((x)->u.symbol.macro)
#define symbol_package(x) ((x)->u.symbol.package)
#define symbol_value(x) ((x)->u.symbol.global_value)
#define type_tag(x) ((x)->u.type.tag)
#define type_name(x) ((x)->u.type.name)
#define vector_last(x) ((x)->u.vector.last)
#define vector_length(x) (x->u.vector.length)
#define vector_value(x) (x->u.vector.value)

/* Opcode accessor macros */
#define opcode_type(x) opcode_name(x)
#define opargn(x, n) (vector_value(opcode_oprands(x))[n])
#define oparg1(x) opargn(x, 0)
#define oparg2(x) opargn(x, 1)
#define oparg3(x) opargn(x, 2)
#define op_argsd_arity(x) oparg1(x)
#define op_args_arity(x) oparg1(x)
#define op_call_arity(x) oparg1(x)
#define op_chktype_pos(x) oparg1(x)
#define op_chktype_type(x) oparg2(x)
#define op_chktype_nargs(x) oparg3(x)
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
