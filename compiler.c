/*
 * compiler.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 *
 * This file contains the definition of the compiler
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
      if (!is_lt_opcode(ins)) {
        writef(standard_out, "ins is %?\n", ins);
        assert(is_lt_opcode(ins));
      }
      if (is_addr_op(ins))
        ins = change_addr(ins, labels);
      if (opcode_name(ins) == FN)
        function_code(op_fn_func(ins)) = assemble(function_code(op_fn_func(ins)));
      vector_value(code_vector)[index] = ins;
      vector_last(code_vector)++;
      index++;
    }
    code = pair_tail(code);
  }
  return code_vector;
}

lisp_object_t *assemble(lisp_object_t *code) {
  assert(is_lt_pair(code));
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
    case CALL:
      ins = make_op_call(va_arg(ap, lisp_object_t *));
      break;
    case CATCH:
      ins = make_op_catch();
      break;
    case CHECKEX:
      ins = make_op_checkex();
      break;
    case CHKARITY: ins = make_op_chkarity(va_arg(ap, lt *)); break;
    case CHKTYPE: {
      lt *position = va_arg(ap, lt *);
      lt *type = va_arg(ap, lt *);
      lt *nargs = va_arg(ap, lt *);
      ins = make_op_chktype(position, type, nargs);
    }
      break;
    case CONST: {
      lisp_object_t *value = va_arg(ap, lisp_object_t *);
      ins = make_op_const(value);
    }
      break;
    case CUTSTACK: ins = make_op_cutstack(); break;
    case EXTENV: {
      lt *count = va_arg(ap, lt *);
      ins = make_op_extenv(count);
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
    case MOVEARGS: {
      lt *count = va_arg(ap, lt *);
      ins = make_op_moveargs(count);
    }
      break;
    case MVLIST: ins = make_op_mvlist(); break;
    case POP:
      ins = make_op_pop();
      break;
    case POPENV:
      ins = make_op_popenv();
      break;
    case PRIM:
      ins = make_op_prim(va_arg(ap, lisp_object_t *));
      break;
    case RESTARGS:
      ins = make_op_restargs(va_arg(ap, lt *));
      break;
    case RETURN:
      ins = make_op_return();
      break;
    case SETMV: ins = make_op_setmv(); break;
    case VALUES: ins = make_op_values(va_arg(ap, lt *)); break;
    default:
      fprintf(stdout, "Invalid opcode %d\n", opcode);
      exit(1);
  }
  return make_pair(ins, make_empty_list());
}

int is_all_symbol(lisp_object_t *list) {
  while (!isnull(list)) {
    if (!is_lt_symbol(pair_head(list)))
      return FALSE;
    list = pair_tail(list);
  }
  return TRUE;
}

int islength1(lisp_object_t *list) {
  return isnull(pair_tail(list));
}

// The order of arguments pushed onto operand stack is the same as the order of
// formal parameters list scanned from left to right. Therefore, the left most
// argument would be pushed to operand stack first.
// The order of arguments on the operand stack from top to bottom, is opposite
// to the order of arguments in environment binding from left to right.
lisp_object_t *compile_args(lisp_object_t *args, lisp_object_t *env) {
  if (isnull(args))
    return the_empty_list;
  else {
    lt *arg = compile_object(pair_head(args), env);
    if (is_signaled(arg))
      return arg;
    else
      return append2(arg, compile_args(pair_tail(args), env));
  }
}

lisp_object_t *compile_begin(lisp_object_t *exps, lisp_object_t *env) {
  if (isnull(exps))
    return gen(CONST, the_empty_list);
  else if (islength1(exps))
    return compile_object(first(exps), env);
  else {
    lisp_object_t *st = compile_object(first(exps), env);
    lt *nd = gen(POP);
    lisp_object_t *rd = compile_begin(pair_tail(exps), env);
    return seq(st, nd, rd);
  }
}

lt *gen_args(lt *args, int nrequired) {
  if (isnull(args))
    return seq(gen(CHKARITY, make_fixnum(nrequired)),
        gen(MOVEARGS, make_fixnum(nrequired)));
  else if (is_lt_symbol(args)) {
    return seq(gen(RESTARGS, make_fixnum(nrequired)),
        gen(CHKARITY, make_fixnum(nrequired)),
        gen(MOVEARGS, make_fixnum(nrequired + 1)));
  } else if (is_lt_pair(args) && is_lt_symbol(pair_head(args))) {
    return gen_args(pair_tail(args), nrequired + 1);
  } else {
    printf("Illegal argument list");
    exit(1);
  }
}

lt *make_proper_args(lt *args) {
  if (isnull(args))
    return make_empty_list();
  else if (!is_lt_pair(args))
    return list1(args);
  else
    return make_pair(pair_head(args), make_proper_args(pair_tail(args)));
}

lt *compile_lambda(lt *args, lt *body, lt *env) {
  lt *arg_ins = gen_args(args, 0);
  env = make_environment(make_proper_args(args), env);
  lisp_object_t *code =
      seq(arg_ins,
          compile_begin(body, env),
          gen(RETURN));
  lisp_object_t *func = make_function(args, code, env);
  return gen(FN, func);
}

lisp_object_t *make_label(void) {
  static int label_count = 1;
  static char buffer[256];
  int i = sprintf(buffer, "L%d", label_count);
  label_count++;
  return S(strndup(buffer, i));
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
  assert(isnull(bindings) || is_lt_pair(bindings));
  if (is_lt_pair(bindings)) {
    int j = 0;
    while (!isnull(bindings)) {
      if (!isfalse(lt_eq(var, pair_head(bindings))))
        return make_fixnum(j);
      bindings = pair_tail(bindings);
      j++;
    }
  }
  return NULL;
}

lisp_object_t *is_var_in_env(lisp_object_t *symbol, lisp_object_t *env) {
  assert(is_lt_symbol(symbol));
  assert(is_lt_environment(env) || isnull_env(env));
  int i = 0;
  while (!isnull_env(env)) {
    lt *bindings = environment_bindings(env);
    assert(isnull(bindings) || is_lt_pair(bindings) || is_lt_vector(bindings));
    lisp_object_t *j = is_var_in_frame(symbol, bindings);
    if (j != NULL)
      return make_pair(make_fixnum(i), j);
    env = environment_next(env);
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
  assert(is_lt_environment(env) || isnull_env(env));
  lisp_object_t *co = is_var_in_env(symbol, env);
  if (co == NULL)
    return gen(GVAR, symbol);
  else {
    lisp_object_t *i = pair_head(co);
    lisp_object_t *j = pair_tail(co);
    return gen(LVAR, i, j, symbol);
  }
}

int is_primitive_fun_name(lt *variable, lt *env) {
  if (is_lt_symbol(variable) && is_var_in_env(variable, env))
    return FALSE;
  return is_lt_symbol(variable) &&
      is_symbol_bound(variable) &&
      is_lt_primitive(symbol_value(variable));
}

lt *compile_type_check(lt *prim, lt *nargs) {
  lt *sig = primitive_signature(prim);
  assert(is_lt_pair(sig) || isnull(sig));
  lt *seq = make_empty_list();
  int i = 0;
  while (!isnull(sig)) {
    lt *pred = pair_head(sig);
    seq = append2(gen(CHKTYPE, make_fixnum(i), pred, nargs), seq);
    sig = pair_tail(sig);
    i++;
  }
  return seq;
}

int is_argc_satisfy(int argc, lt *prim_name) {
  lt *fn = symbol_value(prim_name);
  assert(is_lt_primitive(fn));
  int arity = primitive_arity(fn);
  if (primitive_restp(fn)) {
    arity--;
    return argc >= arity;
  } else
    return argc == arity;
}

lt *compiler_error(char *message) {
  return make_exception(message, TRUE, the_compiler_error_symbol, the_empty_list);
}

lt *compile_tagbody(lt *forms, lt *env) {
  if (isnull(forms))
    return make_empty_list();
  else {
    lt *form = pair_head(forms);
    if (is_lt_symbol(form))
      return seq(list1(form), compile_tagbody(pair_tail(forms), env));
    else {
      lt *part1 = compile_object(form, env);
      return seq(part1, compile_tagbody(pair_tail(forms), env));
    }
  }
}

int is_single_gvar(lt *seq) {
  return isnull(pair_tail(seq)) && opcode_type(pair_head(seq)) == GVAR;
}

lt *compile_checkex(void) {
  return is_check_exception? gen(CHECKEX): the_empty_list;
}

lt *compile_app(lt *proc, lt *args, lt *env) {
  lt *nargs = make_fixnum(pair_length(args));
  lt *op = compile_object(proc, env);
  args = compile_args(args, env);
  if (is_signaled(args))
    return args;
  if (is_primitive_fun_name(proc, env)) {
    lt *prim = symbol_value(proc);
    if (isopcode_fn(proc))
      return seq(args,
          compile_type_check(prim, nargs),
          make_fn_inst(proc),
          compile_checkex(),
          gen(CUTSTACK));
    else
      return seq(args,
          compile_type_check(prim, nargs),
          op,
          gen(PRIM, nargs),
          compile_checkex(),
          gen(CUTSTACK));
  } else
    return seq(args,
        op,
        gen(CALL, nargs),
        compile_checkex(),
        gen(CUTSTACK));
}

lt *compile_mvlist(lt *arg, lt *env) {
  arg = compile_object(arg, env);
  return seq(gen(SETMV),
      arg,
      gen(MVLIST));
}

lt *compile_return(lt *value, lt *env) {
  value = compile_object(value, env);
  return seq(value, gen(RETURN));
}

lt *compile_values(lt *args, lt *env) {
  assert(isnull(args) || is_lt_pair(args));
  assert(is_lt_environment(env));
  int len = 0;
  lt *is = make_empty_list();
  while (is_lt_pair(args)) {
    lt *arg = pair_head(args);
    arg = compile_object(arg, env);
    pair_tail(arg) = is;
    is = arg;
    args = pair_tail(args);
    len++;
  }
  is = lt_list_nreverse(is);
  return seq(is,
      gen(VALUES, make_fixnum(len)),
      gen(RETURN));
}

lt *check_let(lt *form) {
  if (!is_lt_pair(pair_tail(form)))
    return compiler_error("Not well-form LET, no arguments.");
  lt *bds = second(form);
  if (!is_lt_pair(bds) && !isnull(bds))
    return compiler_error("Not well-form LET bindings, must be a list.");
  while (is_lt_pair(bds)) {
    lt *bd = pair_head(bds);
    if (!is_lt_pair(bd))
      return compiler_error("Not well-form LET bindings, must be a A-list.");
    bds = pair_tail(bds);
    if (!is_lt_pair(bds) && !is_lt_pair(bds))
      return compiler_error("No well-form LET bindings, must be a proper list.");
  }
  return the_true;
}

lt *compile_let_bindings(lt *vals, lt *env) {
  return compile_args(vals, env);
}

lt *compile_let(lt *form, lt *env) {
//  Check the syntax of LET form
  lt *res = check_let(form);
  if (is_signaled(res))
    return res;
  lt *bindings = let_bindings(form);
  lt *body = let_body(form);
  lt *vars = let_vars(bindings);
  lt *vals = let_vals(bindings);
  lt *count = make_fixnum(pair_length(vars));
  env = make_environment(vars, env);
  return seq(gen(EXTENV, count),
      compile_let_bindings(vals, env),
      gen(MOVEARGS, count),
      compile_begin(body, env),
      gen(POPENV));
}

lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env) {
  if (is_lt_symbol(object))
    return gen_var(object, env);
  if (!is_lt_pair(object))
    return gen(CONST, object);
  if (is_macro_form(object))
    return compile_object(lt_expand_macro(object), env);
  if (is_let_form(object))
    return compile_let(object, env);
  if (is_quote_form(object)) {
    if (pair_length(object) != 2)
      return compiler_error("There must and be only one argument of a quote form");
    return gen(CONST, second(object));
  }
  if (is_begin_form(object))
    return compile_begin(pair_tail(object), env);
  if (is_set_form(object)) {
    if (pair_length(object) != 3)
      return compiler_error("There must and be only two arguments of a set! form");
    if (!is_lt_symbol(second(object)))
      return compiler_error("The variable as the first variable must be of type symbol");
    lisp_object_t *value = compile_object(third(object), env);
    lisp_object_t *set = gen_set(second(object), env);
    return seq(value, set);
  }
  if (is_if_form(object)) {
    int len = pair_length(object);
    if (!(3 <= len && len <= 4))
      return compiler_error("The number of arguments of a if form must between 3 and 4");
    lisp_object_t *pred = second(object);
    lisp_object_t *then = third(object);
    lisp_object_t *else_part = fourth(object);
    return compile_if(pred, then, else_part, env);
  }
  if (is_lambda_form(object))
    return compile_lambda(second(object), pair_tail(pair_tail(object)), env);
  if (is_mvl_form(object))
    return compile_mvlist(second(object), env);
  if (is_catch_form(object))
    return gen(CATCH);
  if (is_goto_form(object))
    return gen(JUMP, second(object));
  if (is_return_form(object))
    return compile_return(second(object), env);
  if (is_tagbody_form(object))
    return compile_tagbody(pair_tail(object), env);
  if (is_values_form(object))
    return compile_values(pair_tail(object), env);
  if (is_lt_pair(object)) {
    lt *args = pair_tail(object);
    lisp_object_t *fn = pair_head(object);
    return compile_app(fn, args, env);
  }
  writef(standard_out, "Impossible --- Unable to compile %?\n", object);
  exit(1);
}

lt *compile_to_bytecode(lt *form) {
  lt *x = compile_object(form, null_env);
  if (is_signaled(x))
    return x;
  else
    return assemble(x);
}
