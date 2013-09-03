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
      lt *nargs = va_arg(ap, lt *);
      ins = make_op_chktype(position, type, nargs);
    }
      break;
    case CONST: {
      lisp_object_t *value = va_arg(ap, lisp_object_t *);
      ins = make_op_const(value);
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
    case MVCALL:
      ins = make_op_mvcall();
      break;
    case NEED:
      ins = make_op_need();
      break;
    case POP:
      ins = make_op_pop();
      break;
    case PRIM:
      ins = make_op_prim(va_arg(ap, lisp_object_t *));
      break;
    case RETURN:
      ins = make_op_return(va_arg(ap, lt *));
      break;
    case VALUES:
      ins = make_op_values(va_arg(ap, lt *));
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
  else {
    lt *arg = compile_object(pair_head(args), env);
    if (is_signaled(arg))
      return arg;
    else
      return append2(arg, compile_args(pair_tail(args), env));
  }
}

int is_no_value_return(lt *inseq) {
  lt *ins = lt_list_last(inseq);
  return opcode_type(ins) == RETURN;
}

lisp_object_t *compile_begin(lisp_object_t *exps, lisp_object_t *env) {
  if (isnull(exps))
    return gen(CONST, the_empty_list);
  else if (islength1(exps))
    return compile_object(first(exps), env);
  else {
    lisp_object_t *st = compile_object(first(exps), env);
    lisp_object_t *nd = is_no_value_return(st) ? the_empty_list: gen(POP);
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
  lt *arg_ins = gen_args(args, 0);
  env = make_environment(make_proper_args(args), env);
  lisp_object_t *code =
      seq(arg_ins,
          compile_begin(body, env),
          gen(RETURN, the_false));
  lisp_object_t *func = make_function(env, args, code, null_env);
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
  assert(isenvironment(env) || isnull_env(env));
  int i = 0;
  while (!isnull_env(env)) {
    lt *bindings = environment_bindings(env);
    assert(isnull(bindings) || ispair(bindings) || isvector(bindings));
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
  assert(isenvironment(env) || isnull_env(env));
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
  if (issymbol(variable) && is_var_in_env(variable, env))
    return FALSE;
  return issymbol(variable) &&
      is_symbol_bound(variable) &&
      isprimitive(symbol_value(variable));
}

lt *add_local_var(lt *var, lt *env) {
  if (isnull_env(env))
    return env;
  assert(ispair(environment_bindings(env)) || isnull(environment_bindings(env)));
  if (ispair(environment_bindings(env))) {
    lt *tmp = environment_bindings(env);
    while (ispair(tmp)) {
      if (pair_head(tmp) == var)
        return env;
      tmp = pair_tail(tmp);
    }
    lt *c = list1(var);
    environment_bindings(env) = seq(environment_bindings(env), c);
    return env;
  } else {
    environment_bindings(env) = list1(var);
    return env;
  }
}

lt *compile_type_check(lt *prim, lt *nargs) {
  lt *sig = primitive_signature(prim);
  assert(ispair(sig) || isnull(sig));
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
  assert(isprimitive(fn));
  int arity = primitive_arity(fn);
  if (primitive_restp(fn)) {
    arity--;
    return argc >= arity;
  } else
    return argc == arity;
}

lt *compiler_error(char *message) {
  return make_exception(message, TRUE, S("COMPILER-ERROR"), the_empty_list);
}

lt *compile_tagbody(lt *forms, lt *env) {
  if (isnull(forms))
    return make_empty_list();
  else {
    lt *form = pair_head(forms);
    if (issymbol(form))
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

lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env) {
  if (issymbol(object))
    return gen_var(object, env);
  if (!ispair(object))
    return gen(CONST, object);
  if (is_macro_form(object))
    return compile_object(lt_expand_macro(object), env);
//  if (is_tag_list(object, S("quote"))) {
  if (is_quote_form(object)) {
    if (pair_length(object) != 2)
      return compiler_error("There must and be only one argument of a quote form");
    return gen(CONST, second(object));
  }
//  if (is_tag_list(object, S("begin")))
  if (is_begin_form(object))
    return compile_begin(pair_tail(object), env);
//  if (is_tag_list(object, S("set!"))) {
  if (is_set_form(object)) {
    if (pair_length(object) != 3)
      return compiler_error("There must and be only two arguments of a set! form");
    if (!issymbol(second(object)))
      return compiler_error("The variable as the first variable must be of type symbol");
    lisp_object_t *value = compile_object(third(object), env);
    lisp_object_t *set = gen_set(second(object), env);
    return seq(value, set);
  }
//  if (is_tag_list(object, S("if"))) {
  if (is_if_form(object)) {
    int len = pair_length(object);
    if (!(3 <= len && len <= 4))
      return compiler_error("The number of arguments of a if form must between 3 and 4");
    lisp_object_t *pred = second(object);
    lisp_object_t *then = third(object);
    lisp_object_t *else_part = fourth(object);
    return compile_if(pred, then, else_part, env);
  }
//  if (is_tag_list(object, S("lambda")))
  if (is_lambda_form(object))
    return gen(FN, compile_lambda(second(object), pair_tail(pair_tail(object)), env));
//  if (is_tag_list(object, S("catch")))
  if (is_catch_form(object))
    return gen(CATCH);
//  if (is_tag_list(object, S("var"))) {
  if (is_var_form(object)) {
    env = add_local_var(second(object), env);
    return compile_object(make_pair(S("set!"), pair_tail(object)), env);
  }
//  if (is_tag_list(object, S("goto")))
  if (is_goto_form(object))
    return gen(JUMP, second(object));
//  if (is_tag_list(object, S("tagbody"))) {
  if (is_tagbody_form(object)) {
    return compile_tagbody(pair_tail(object), env);
  }
//  if (is_tag_list(object, S("call-with-values"))) {
  if (is_cwv_form(object)) {
    lt *fn = compile_object(second(object), env);
    lt *vals = compile_object(third(object), env);
    return seq(gen(NEED), vals, fn, gen(MVCALL), gen(CHECKEX));
  }
//  if (is_tag_list(object, S("values"))) {
  if (is_values_form(object)) {
    lt *args = compile_args(pair_tail(object), env);
    return seq(args, gen(VALUES, lt_list_length(pair_tail(object))));
  }
//  if (is_tag_list(object, S("yield"))) {
  if (is_yield_form(object)) {
    lt *val = second(object);
    return seq(compile_object(val, env), gen(RETURN, the_true));
  }
  if (ispair(object)) {
    lisp_object_t *args = pair_tail(object);
    lisp_object_t *fn = pair_head(object);
    lt *op = compile_object(fn, env);
    if (is_single_gvar(op) && isundef(symbol_value(fn)))
      writef(standard_error, "Warning: Function named %S is undefined.\n", fn);
    /* Generating different instruction when calling primitive and anything else */
    if (is_primitive_fun_name(fn, env)) {
      lt *nargs = lt_list_length(args);
      if (!is_argc_satisfy(fixnum_value(nargs), fn))
        return compiler_error("The number of arguments passed to primitive function is wrong");
      args = compile_args(args, env);
      if (is_signaled(args))
        return args;
      return seq(args,
                 compile_type_check(symbol_value(fn), nargs),
                 op,
                 gen(PRIM, nargs),
                 (is_check_type? gen(CHECKEX): the_empty_list));
    } else
      return seq(compile_args(args, env),
                 op,
                 gen(CALL, lt_list_length(args)),
                 (is_check_type? gen(CHECKEX): the_empty_list));
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
