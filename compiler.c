/*
 * compiler.c
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
#include "prims.h"
#include "type.h"
#include "utilities.h"

lt *assemble(lt *);
lt *compile_object(lt *, lt *);

int is_addr_op(lt *op) {
  switch (opcode_type(op)) {
    case JUMP: case FJUMP:
      return TRUE;
    default :
      return FALSE;
  }
}

lt *get_offset(lt *label, lt *labels) {
  lt *last_labels = labels;
  while (!isnull(labels)) {
    lt *lo = pair_head(labels);
    if (!isfalse(lt_eq(label, pair_head(lo))))
      return pair_tail(lo);
    labels = pair_tail(labels);
  }
  fprintf(stdout, "Impossible!!! %s not found!\n", symbol_name(label));
//  write_expr("labels", last_labels);
  writef(standard_out, "labels => %?\n", last_labels);
  exit(1);
}

lt *change_addr(lt *ins, lt *table) {
  switch (opcode_type(ins)) {
    case FJUMP: {
      lt *label = op_fjump_label(ins);
      return make_op_fjump(get_offset(label, table));
    }
    case JUMP: {
      lt *label = op_jump_label(ins);
      return make_op_jump(get_offset(label, table));
    }
    default :
      fprintf(stdout, "Invalid instruction to change address\n");
      exit(1);
  }
}

lt *asm_first_pass(lt *code) {
  int length = 0;
  lt *labels = the_empty_list;
  while (!isnull(code)) {
    lt *ins = pair_head(code);
    if (is_label(ins))
      labels = make_pair(make_pair(ins, make_fixnum(length)), labels);
    else
      length++;
    code = pair_tail(code);
  }
  return make_pair(make_fixnum(length), labels);
}

lt *asm_second_pass(lt *code, lt *length, lt *labels) {
  int index = 0;
  lisp_object_t *code_vector = make_vector(fixnum_value(length));
  while (!isnull(code)) {
    lisp_object_t *ins = pair_head(code);
    if (!is_label(ins)) {
      assert(isopcode(ins));
      if (is_addr_op(ins))
        ins = change_addr(ins, labels);
      if (opcode_name(ins) == FN)
        function_code(op_fn_func(ins)) = assemble(function_code(op_fn_func(ins)));
      if (opcode_name(ins) == MACROFN) {
        lt *func = op_macro_func(ins);
        assert(isfunction(func));
        function_code(func) = assemble(function_code(func));
      }
      vector_value(code_vector)[index] = ins;
      vector_last(code_vector)++;
      index++;
    }
    code = pair_tail(code);
  }
  return code_vector;
}

lisp_object_t *assemble(lisp_object_t *code) {
  assert(ispair(code));
  lisp_object_t *ll = asm_first_pass(code);
  lisp_object_t *length = pair_head(ll);
  lisp_object_t *labels = pair_tail(ll);
  code = asm_second_pass(code, length, labels);
  return code;
}

lisp_object_t *gen(enum TYPE opcode, ...) {
  va_list ap;
  va_start(ap, opcode);
  lisp_object_t *ins;
  switch (opcode) {
    case ARGSD:
      ins = make_op_argsd(va_arg(ap, lt *));
      break;
    case ARGS: {
      lisp_object_t *length = va_arg(ap, lisp_object_t *);
      ins = make_op_args(length);
    }
      break;
    case CALL:
      ins = make_op_call(va_arg(ap, lisp_object_t *));
      break;
    case CATCH:
      ins = make_op_catch();
      break;
    case CHECKEX:
      ins = make_op_checkex();
      break;
    case CHKTYPE: {
      lt *position = va_arg(ap, lt *);
      lt *type = va_arg(ap, lt *);
      ins = make_op_chktype(position, type);
    }
      break;
    case CONST: {
      lisp_object_t *value = va_arg(ap, lisp_object_t *);
      ins = make_op_const(value);
    }
      break;
    case DECL: {
      lt *symbol = va_arg(ap, lt *);
      ins = make_op_decl(symbol);
    }
      break;
    case FJUMP: {
      lisp_object_t *label = va_arg(ap, lisp_object_t *);
      ins = make_op_fjump(label);
    }
      break;
    case FN:
      ins = make_op_fn(va_arg(ap, lisp_object_t *));
      break;
    case GSET:
      ins = make_op_gset(va_arg(ap, lisp_object_t *));
      break;
    case GVAR:
      ins = make_op_gvar(va_arg(ap, lisp_object_t *));
      break;
    case JUMP: {
      lisp_object_t *label = va_arg(ap, lisp_object_t *);
      ins = make_op_jump(label);
    }
      break;
    case LSET: {
      lisp_object_t *i = va_arg(ap, lisp_object_t *);
      lisp_object_t *j = va_arg(ap, lisp_object_t *);
      lisp_object_t *symbol = va_arg(ap, lisp_object_t *);
      ins = make_op_lset(i, j, symbol);
    }
      break;
    case LVAR: {
      lisp_object_t *i = va_arg(ap, lisp_object_t *);
      lisp_object_t *j = va_arg(ap, lisp_object_t *);
      lisp_object_t *symbol = va_arg(ap, lisp_object_t *);
      ins = make_op_lvar(i, j, symbol);
    }
      break;
    case MACROFN:
      ins = make_op_macro(va_arg(ap, lt *));
      break;
    case POP:
      ins = make_op_pop();
      break;
    case PRIM:
      ins = make_op_prim(va_arg(ap, lisp_object_t *));
      break;
    case RETURN:
      ins = make_op_return();
      break;
    default:
      fprintf(stdout, "Invalid opcode %d\n", opcode);
      exit(1);
  }
  return make_pair(ins, make_empty_list());
}

int is_all_symbol(lisp_object_t *list) {
  while (!isnull(list)) {
    if (!issymbol(pair_head(list)))
      return FALSE;
    list = pair_tail(list);
  }
  return TRUE;
}

int islength1(lisp_object_t *list) {
  return isnull(pair_tail(list));
}

lisp_object_t *compile_args(lisp_object_t *args, lisp_object_t *env) {
  if (isnull(args))
    return the_empty_list;
  else
    return append2(compile_object(pair_head(args), env),
                      compile_args(pair_tail(args), env));
}

lisp_object_t *compile_begin(lisp_object_t *exps, lisp_object_t *env) {
  if (isnull(exps))
    return gen(CONST, the_empty_list);
  else if (islength1(exps))
    return compile_object(first(exps), env);
  else {
    lisp_object_t *st = compile_object(first(exps), env);
    lisp_object_t *nd = gen(POP);
    lisp_object_t *rd = compile_begin(pair_tail(exps), env);
    return seq(st, nd, rd);
  }
}

lt *gen_args(lt *args, int nrequired) {
  if (isnull(args))
    return gen(ARGS, make_fixnum(nrequired));
  else if (issymbol(args)) {
    return gen(ARGSD, make_fixnum(nrequired));
  } else if (ispair(args) && issymbol(pair_head(args))) {
    return gen_args(pair_tail(args), nrequired + 1);
  } else {
    printf("Illegal argument list");
    exit(1);
  }
}

lt *make_proper_args(lt *args) {
  if (isnull(args))
    return make_empty_list();
  else if (!ispair(args))
    return list1(args);
  else
    return make_pair(pair_head(args), make_proper_args(pair_tail(args)));
}

lt *compile_lambda(lt *args, lt *body, lt *env) {
//  assert(is_all_symbol(args));
//  lisp_object_t *len = lt_list_length(args);
  lt *arg_ins = gen_args(args, 0);
  lisp_object_t *code =
      seq(arg_ins,
          compile_begin(body, make_pair(make_proper_args(args), env)),
          gen(RETURN));
  lisp_object_t *func = make_function(env, args, code);
  return func;
}

lisp_object_t *make_label(void) {
  static int label_count = 1;
  static char buffer[256];
  int i = sprintf(buffer, "L%d", label_count);
  label_count++;
  return find_or_create_symbol(strndup(buffer, i));
}

lt *compile_if(lt *pred, lt *then, lt *else_part, lt *env) {
  lisp_object_t *l1 = make_label();
  lisp_object_t *l2 = make_label();
  pred = compile_object(pred, env);
  then = compile_object(then, env);
  else_part = compile_object(else_part, env);
  lisp_object_t *fj = gen(FJUMP, l1);
  lisp_object_t *j = gen(JUMP, l2);
  return seq(pred, fj, then, j, list1(l1), else_part, list1(l2));
}

lisp_object_t *is_var_in_frame(lisp_object_t *var, lisp_object_t *bindings) {
  assert(isnull(bindings) || ispair(bindings) || isvector(bindings));
  if (ispair(bindings)) {
    int j = 0;
    while (!isnull(bindings)) {
      if (!isfalse(lt_eq(var, pair_head(bindings))))
        return make_fixnum(j);
      bindings = pair_tail(bindings);
      j++;
    }
  }
  if (isvector(bindings)) {
    printf("Searching %s in vector...\n", symbol_name(var));
    for (int j = 0; j < vector_length(bindings); j++) {
      if (!isfalse(lt_eq(var, vector_value(bindings)[j])))
        return make_fixnum(j);
    }
  }
  return NULL;
}

lisp_object_t *is_var_in_env(lisp_object_t *symbol, lisp_object_t *env) {
  assert(issymbol(symbol));
  int i = 0;
  while (env != null_env) {
    lisp_object_t *bindings = pair_head(env);
    assert(isnull(bindings) || ispair(bindings) || isvector(bindings));
    lisp_object_t *j = is_var_in_frame(symbol, bindings);
    if (j != NULL)
      return make_pair(make_fixnum(i), j);
    env = pair_tail(env);
    i++;
  }
  return NULL;
}

lisp_object_t *gen_set(lisp_object_t *symbol, lisp_object_t *env) {
  lisp_object_t *co = is_var_in_env(symbol, env);
  if (co == NULL)
    return gen(GSET, symbol);
  else {
    lisp_object_t *i = pair_head(co);
    lisp_object_t *j = pair_tail(co);
    return gen(LSET, i, j, symbol);
  }
}

lisp_object_t *gen_var(lisp_object_t *symbol, lisp_object_t *env) {
  lisp_object_t *co = is_var_in_env(symbol, env);
  if (co == NULL)
    return gen(GVAR, symbol);
  else {
    lisp_object_t *i = pair_head(co);
    lisp_object_t *j = pair_tail(co);
    return gen(LVAR, i, j, symbol);
  }
}

int is_primitive_fun(lt *variable) {
  return issymbol(variable) &&
      is_symbol_bound(variable) &&
      isprimitive(symbol_value(variable));
}

void add_local_var(lt *var, lt *env) {
  if (env == null_env)
    return;
  assert(ispair(pair_head(env)) || isnull(pair_head(env)));
  if (ispair(pair_head(env))) {
    lt *c = list1(var);
    pair_head(env) = seq(pair_head(env), c);
  } else {
    pair_head(env) = list1(var);
  }
}

/* TODO: The support for tail call optimization. */
pub lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env) {
  if (issymbol(object))
    return gen_var(object, env);
  if (!ispair(object))
    return gen(CONST, object);
  if (is_macro_form(object))
    return compile_object(lt_expand_macro(object), env);
  if (is_tag_list(object, S("quote")))
    return gen(CONST, second(object));
  if (is_tag_list(object, S("begin")))
    return compile_begin(pair_tail(object), env);
  if (is_tag_list(object, S("set!"))) {
    lisp_object_t *value = compile_object(third(object), env);
    lisp_object_t *set = gen_set(second(object), env);
    return seq(value, set);
  }
  if (is_tag_list(object, S("if"))) {
    lisp_object_t *pred = second(object);
    lisp_object_t *then = third(object);
    lisp_object_t *else_part = fourth(object);
    return compile_if(pred, then, else_part, env);
  }
  if (is_tag_list(object, S("lambda")))
    return gen(FN, compile_lambda(second(object), pair_tail(pair_tail(object)), env));
  if (is_tag_list(object, S("macro"))) {
    lt *proc = compile_lambda(second(object), pair_tail(pair_tail(object)), env);
    return gen(MACROFN, proc);
  }
  if (is_tag_list(object, S("catch")))
    return gen(CATCH);
  if (is_tag_list(object, S("declare"))) {
    add_local_var(second(object), env);
    return gen(DECL, second(object));
  }
  if (ispair(object)) {
    lisp_object_t *args = pair_tail(object);
    lisp_object_t *fn = pair_head(object);
    /* Generating different instruction when calling primitive and anything else */
    if (is_primitive_fun(fn))
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(PRIM, lt_list_length(args)),
                 gen(CHECKEX));
    else
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(CALL, lt_list_length(args)),
                 gen(CHECKEX));
  }
  writef(standard_out, "Impossible --- Unable to compile %?\n", object);
  exit(1);
}

lt *compile_to_bytecode(lt *form) {
  return assemble(compile_object(form, null_env));
}
