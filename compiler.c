/*
 * compiler.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */
#include "type.h"
#include "object.h"

/* PART: compiler.c */
/* Compiler */
int is_label(lisp_object_t *object) {
  return issymbol(object);
}

int is_addr_op(lisp_object_t *op) {
  switch (opcode_type(op)) {
    case JUMP: case FJUMP: return TRUE;
    default : return FALSE;
  }
}

lisp_object_t *get_offset(lisp_object_t *label, lisp_object_t *labels) {
  lisp_object_t *last_labels = labels;
  while (!isnull(labels)) {
    lisp_object_t *lo = pair_head(labels);
    if (!isfalse(lt_eq(label, pair_head(lo))))
      return pair_tail(lo);
    labels = pair_tail(labels);
  }
  fprintf(stdout, "Impossible!!! %s not found!\n", symbol_name(label));
  write_expr("labels", last_labels);
  exit(1);
}

lisp_object_t *change_addr(lisp_object_t *ins, lisp_object_t *table) {
  switch (opcode_type(ins)) {
    case FJUMP: {
      lisp_object_t *label = op_fjump_label(ins);
      return make_op_fjump(get_offset(label, table));
    }
    case JUMP: {
      lisp_object_t *label = op_jump_label(ins);
      return make_op_jump(get_offset(label, table));
    }
    default :
      fprintf(stdout, "Invalid instruction to change address\n");
      exit(1);
  }
}

lisp_object_t *asm_first_pass(lisp_object_t *code) {
  int length = 0;
  lisp_object_t *labels = null_list;
  while (!isnull(code)) {
    lisp_object_t *ins = pair_head(code);
    if (is_label(ins))
      labels = make_pair(make_pair(ins, make_fixnum(length)), labels);
    else
      length++;
    code = pair_tail(code);
  }
  return make_pair(make_fixnum(length), labels);
}

lisp_object_t *asm_second_pass(lisp_object_t *code, lisp_object_t *length, lisp_object_t *labels) {
  int index = 0;
  lisp_object_t *code_vector = make_vector(fixnum_value(length));
  while (!isnull(code)) {
    lisp_object_t *ins = pair_head(code);
    if (!is_label(ins)) {
      if (is_addr_op(ins))
        ins = change_addr(ins, labels);
      vector_value(code_vector)[index] = ins;
      vector_last(code_vector)++;
      index++;
    }
    code = pair_tail(code);
  }
  return code_vector;
}

lisp_object_t *assemble(lisp_object_t *func) {
  lisp_object_t *ll = asm_first_pass(function_code(func));
  lisp_object_t *length = pair_head(ll);
  lisp_object_t *labels = pair_tail(ll);
  function_code(func) = asm_second_pass(function_code(func), length, labels);
  return func;
}

lisp_object_t *gen(enum TYPE opcode, ...) {
  va_list ap;
  va_start(ap, opcode);
  lisp_object_t *ins;
  switch (opcode) {
    case ARGS: {
      lisp_object_t *length = va_arg(ap, lisp_object_t *);
      ins = make_op_args(length);
    }
      break;
    case CALL: ins = make_op_call(va_arg(ap, lisp_object_t *)); break;
    case CATCH: {
      lt *type_name = va_arg(ap, lt *);
      lt *handler = va_arg(ap, lt *);
      ins = make_op_catch(type_name, handler);
    }
      break;
    case FJUMP: {
      lisp_object_t *label = va_arg(ap, lisp_object_t *);
      ins = make_op_fjump(label);
    }
      break;
    case FN: ins = make_op_fn(va_arg(ap, lisp_object_t *)); break;
    case GSET: ins = make_op_gset(va_arg(ap, lisp_object_t *)); break;
    case GVAR: ins = make_op_gvar(va_arg(ap, lisp_object_t *)); break;
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
    case CONST: {
      lisp_object_t *value = va_arg(ap, lisp_object_t *);
      ins = make_op_const(value);
    }
      break;
    case POP: ins = make_op_pop(); break;
    case PRIM: ins = make_op_prim(va_arg(ap, lisp_object_t *)); break;
    case RETURN: ins = make_op_return(); break;
    default :
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

#define first(x) lt_raw_nth((x), 0)
#define second(x) lt_raw_nth((x), 1)
#define third(x) lt_raw_nth((x), 2)
#define fourth(x) lt_raw_nth((x), 3)

#define seq(...) lt_append(__VA_ARGS__, NULL)

lisp_object_t *compile_args(lisp_object_t *args, lisp_object_t *env) {
  if (isnull(args))
    return null_list;
  else
    return lt_append2(compile_object(pair_head(args), env),
                      compile_args(pair_tail(args), env));
}

lisp_object_t *compile_begin(lisp_object_t *exps, lisp_object_t *env) {
  if (isnull(exps))
    return gen(CONST, null_list);
  else if (islength1(exps))
    return compile_object(first(exps), env);
  else {
    lisp_object_t *st = compile_object(first(exps), env);
    lisp_object_t *nd = gen(POP);
    lisp_object_t *rd = compile_begin(pair_tail(exps), env);
    return seq(st, nd, rd);
  }
}

lisp_object_t *compile_lambda(lisp_object_t *args, lisp_object_t *body, lisp_object_t *env) {
  assert(is_all_symbol(args));
  lisp_object_t *len = lt_list_length(args);
  lisp_object_t *code = seq(gen(ARGS, len),
                            compile_begin(body, make_pair(args, env)),
                            gen(RETURN));
  lisp_object_t *func = make_function(env, args, code);
  return assemble(func);
}

lt *compile_handler(lt *handler, lt *env) {
  assert(ispair(handler));
  assert(ispair(pair_head(handler)));
  lt *type_name = first(pair_head(handler));
  assert(issymbol(type_name));
  lt *var = second(pair_head(handler));
  /* assert(issymbol(var)); */
  assert(ispair(var) && issymbol(pair_head(var)));
  lt *body = pair_tail(handler);
  handler = compile_lambda(var, body, env);
  return gen(CATCH, type_name, handler);
}

lt *compile_handlers(lt *handlers, lt *env) {
  assert(isnull(handlers) || ispair(handlers));
  if (isnull(handlers))
    return null_list;
  else
    return lt_append2(compile_handler(pair_head(handlers), env),
                      compile_handlers(pair_tail(handlers), env));
}

lisp_object_t *make_label(void) {
  static int label_count = 1;
  static char buffer[256];
  int i = sprintf(buffer, "L%d", label_count);
  label_count++;
  return find_or_create_symbol(strndup(buffer, i));
}

lisp_object_t *compile_if(lisp_object_t *pred, lisp_object_t *then, lisp_object_t *else_part, lisp_object_t *env) {
  lisp_object_t *l1 = make_label();
  lisp_object_t *l2 = make_label();
  pred = compile_object(pred, env);
  then = compile_object(then, env);
  else_part = compile_object(else_part, env);
  lisp_object_t *fj = gen(FJUMP, l1);
  lisp_object_t *j = gen(JUMP, l2);
  return seq(pred, fj, then, j, list1(l1), else_part, list1(l2));
}

lt *compile_try_catch(lt *case_list, lt *form, lt *env) {
  return seq(compile_handlers(case_list, env),
             compile_object(form, env));
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
    assert(isnull(bindings) ||ispair(bindings) || isvector(bindings));
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

int is_tag_list(lisp_object_t *object, lisp_object_t *tag) {
  return ispair(object) && (pair_head(object) == tag);
}

/* TODO: The support for built-in macros. */
/* TODO: The support for tail call optimization. */
pub lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env) {
  if (issymbol(object))
    return gen_var(object, env);
  if (!ispair(object))
    return gen(CONST, object);
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
  if (is_tag_list(object, S("try-with"))) {
    lt *form = second(object);
    lt *case_list = pair_tail(pair_tail(object));
    return compile_try_catch(case_list, form, env);
  }
  if (ispair(object)) {
    lisp_object_t *args = pair_tail(object);
    lisp_object_t *fn = pair_head(object);
    assert(issymbol(fn));
    /* Generating different instruction when calling primitive and compiled function */
    if (is_symbol_bound(fn) && isprimitive(symbol_value(fn)))
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(PRIM, lt_list_length(args)));
    else
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(CALL, lt_list_length(args)));
  } else {
    fprintf(stdout, "Invalid expression to be compiled\n");
    exit(1);
  }
}

lisp_object_t *compile_as_lambda(lisp_object_t *form) {
  lisp_object_t *result = compile_lambda(make_empty_list(), list1(form), null_env);
  return result;
}
