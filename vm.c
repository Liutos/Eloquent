/*
 * vm.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 *
 * This file contains the definition of the virtual machine
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "prims.h"
#include "type.h"
#include "utilities.h"

void add_local_variable(lt *var, lt *env) {
  if (isnull_env(env))
    return;
  assert(is_lt_vector(environment_bindings(env)));
  lt_vector_push_extend(environment_bindings(env), make_undef());
}

lt *walk_in_env(lt *env, int n) {
  assert(is_lt_environment(env));
  while (n-- > 0)
    env = environment_next(env);
  return env;
}

lisp_object_t *find_in_frame(lisp_object_t *bindings, int j) {
  assert(is_lt_pair(bindings) || is_lt_vector(bindings));
  if (is_lt_pair(bindings)) {
    bindings = lt_raw_nthtail(bindings, j);
    return pair_head(bindings);
  } else if (is_lt_vector(bindings)) {
    return vector_value(bindings)[j];
  }
  fprintf(stdout, "Impossible!!! variable not found!\n");
  exit(1);
}

lisp_object_t *locate_var(lisp_object_t *env, int i, int j) {
  assert(is_lt_environment(env) || isnull_env(env));
  env = walk_in_env(env, i);
  return find_in_frame(environment_bindings(env), j);
}

lisp_object_t *raw_vector_ref(lisp_object_t *vector, int index) {
  assert(is_lt_vector(vector));
  return vector_value(vector)[index];
}

void set_in_frame(lisp_object_t *bindings, int j, lisp_object_t *value) {
  assert(is_lt_pair(bindings) || is_lt_vector(bindings));
  if (is_lt_pair(bindings)) {
    bindings = lt_raw_nthtail(bindings, j);
    pair_head(bindings) = value;
    return;
  } else if (is_lt_vector(bindings)) {
    vector_value(bindings)[j] = value;
    return;
  }
  fprintf(stdout, "Impossible!!! variable not found!\n");
  exit(1);
}

void set_local_var(lisp_object_t *env, int i, int j, lisp_object_t *value) {
  env = walk_in_env(env, i);
  set_in_frame(environment_bindings(env), j, value);
}

int is_type_satisfy(lt *arg, lt *pred) {
  if (pred == S("object"))
    return TRUE;
  else if (is_lt_type(pred))
    return !isfalse(lt_is_kind_of(arg, pred));
  else if (is_tag_list(pred, S("or"))) {
    lt *types = pair_tail(pred);
    while (is_lt_pair(types)) {
      lt *t = pair_head(types);
      if (is_type_satisfy(arg, t) == TRUE)
        return TRUE;
      else
        types = pair_tail(types);
    }
    return FALSE;
  } else {
    fprintf(stderr, "Unknown type predicate. Please check the signature declarations in function `init_prims' in file `prims.c'.\n");
    exit(1);
  }
}

lt *type_error(lt *index, lt *pred) {
  char msg[256];
  FILE *buf = fmemopen(msg, sizeof(msg), "w");
  lt *file = make_output_port(buf);
  writef(file, "The argument at index %d is not satisfy with predicate %?", index, pred);
  lt_close_out(file);
  return make_exception(strdup(msg), TRUE, the_type_error_symbol, the_empty_list);
}

lt *comp2run_env(lt *comp_env, lt *next) {
  lt *pars = environment_bindings(comp_env);
  assert(is_lt_pair(pars) || isnull(pars));
  int len = pair_length(pars);
  return make_environment(make_vector(len), next);
}

lisp_object_t *run_by_llam(lisp_object_t *code_vector) {
#define _arg(N) vlast(stack, primitive_arity(func) - N)
#define _arg1 _arg(1)
#define _arg2 _arg(2)
#define _arg3 _arg(3)
#define move_stack() vector_last(stack) -= primitive_arity(func)
#define vlast(v, n) lt_vector_last_nth(v, make_fixnum(n))

  lt *stack = make_vector(50);
  lt *arg1;
  lt *arg2;

  assert(is_lt_vector(code_vector));
//  The number of arguments passed.
  int nargs = 0;
  int pc = 0;
  int throw_exception = TRUE;
  lt *code = code_vector;
  lisp_object_t *env = null_env;
  lt *prim = NULL;
  lisp_object_t *return_stack = the_empty_list;
  while (pc < vector_length(code)) {
    lisp_object_t *ins = raw_vector_ref(code, pc);
    if (debug)
      writef(standard_out, "ins is %?\n", ins);
    switch (opcode_type(ins)) {
      case CALL: {
        lisp_object_t *func = lt_vector_pop(stack);
        if (is_lt_primitive(func)) {
//        	This is possible because the first element of a application list
//        	might be a compound expression, and this compound one will return
//        	a primitive function object at run-time, but the compiler is unable
//        	to generate a PRIM instruction at compile-time --- This is only
//        	happen when the first element is a symbol named a primitive function.
        	lt_vector_push(stack, func);
        	goto call_primitive;
        }
        if (!is_lt_function(func)) {
          char msg[1000];
          FILE *fp = fmemopen(msg, sizeof(msg), "w");
          lt *file = make_output_port(fp);
          writef(file, "The object %? at the first place is not a function", func);
          lt_close_out(file);
          return signal_exception(msg);
        }
        lisp_object_t *retaddr =
            make_retaddr(code, env, func, pc, throw_exception, vector_last(stack));
        return_stack = make_pair(retaddr, return_stack);
        code = function_code(func);
        env = function_renv(func);
        pc = -1;
        throw_exception = TRUE;
        nargs = fixnum_value(op_call_arity(ins));
        env = comp2run_env(function_cenv(func), env);
      }
        break;
      case CATCH:
        throw_exception = FALSE;
        lt_vector_push(stack, make_empty_list());
        break;
        check_exception:
      case CHECKEX:
        while (is_signaled(vlast(stack, 0)) && throw_exception) {
          lt *ex = lt_vector_pop(stack);
          if (isnull(return_stack)) {
            if (prim != NULL)
              exception_backtrace(ex) = list1(prim);
            lt_vector_push(stack, ex);
            goto halt;
          }
          lt *ret = pair_head(return_stack);
          lt *fn = retaddr_fn(ret);
          exception_backtrace(ex) = make_pair(fn, exception_backtrace(ex));
          return_stack = pair_tail(return_stack);
          code = retaddr_code(ret);
          env = retaddr_env(ret);
          pc = retaddr_pc(ret);
          throw_exception = retaddr_throw_flag(ret);
          lt_vector_push(stack, ex);
        }
        break;
      case CHKARITY: {
        int arity = fixnum_value(op_chkarity_arity(ins));
        if (arity > nargs)
          return signal_exception("Too few arguments passed");
        if (arity < nargs)
          return signal_exception("Too many arguments passed");
      }
        break;
      case CHKTYPE: {
        lt *index = op_chktype_pos(ins);
        lt *pred = op_chktype_type(ins);
        lt *nargs = op_chktype_nargs(ins);
        lt *arg = vlast(stack, fixnum_value(nargs) - fixnum_value(index) - 1);
        if (is_type_satisfy(arg, pred) == FALSE) {
          return type_error(index, pred);
        }
      }
        break;
      case CONST:
        lt_vector_push(stack, op_const_value(ins));
        break;
      case EXTENV: {
        lt *length = op_extenv_count(ins);
        env = make_environment(make_vector(fixnum_value(length)), env);
      }
        break;
      case FJUMP:
        if (isfalse(lt_vector_pop(stack)))
          pc = fixnum_value(op_fjump_label(ins)) - 1;
        break;
      case FN: {
        lisp_object_t *func = op_fn_func(ins);
        lt *cenv = function_cenv(func);
        func = make_function(cenv, function_args(func), function_code(func), env);
        lt_vector_push(stack, func);
      }
        break;
      case GSET: {
        lisp_object_t *value = vlast(stack, 0);
        lisp_object_t *var = op_gset_var(ins);
        symbol_value(var) = value;
      }
        break;
      case GVAR: {
        lisp_object_t *sym = op_gvar_var(ins);
        if (symbol_value(sym) == the_undef) {
          char msg[256];
          sprintf(msg, "Undefined global variable %s", symbol_name(sym));
          lt_vector_push(stack, signal_exception(strdup(msg)));
          goto check_exception;
        } else
          lt_vector_push(stack, symbol_value(sym));
      }
        break;
      case JUMP: pc = fixnum_value(op_jump_label(ins)) - 1; break;
      case LSET: {
        lisp_object_t *value = vlast(stack, 0);
        lisp_object_t *i = op_lset_i(ins);
        lisp_object_t *j = op_lset_j(ins);
        set_local_var(env, fixnum_value(i), fixnum_value(j), value);
      }
        break;
      case LVAR: {
        lisp_object_t *i = op_lvar_i(ins);
        lisp_object_t *j = op_lvar_j(ins);
        lisp_object_t *value = locate_var(env, fixnum_value(i), fixnum_value(j));
        lt_vector_push(stack, value);
      }
        break;
      case MOVEARGS: {
        lt *bindings = environment_bindings(env);
        for (int i = fixnum_value(op_moveargs_count(ins)) - 1; i >= 0; i--) {
          lt *val = lt_vector_pop(stack);
          vector_value(bindings)[i] = val;
          vector_last(bindings)++;
        }
      }
        break;
      case POP:
        lt_vector_pop(stack);
        break;
      case POPENV:
        env = environment_next(env);
        break;
			call_primitive:
      case PRIM: {
        nargs = fixnum_value(op_prim_nargs(ins));
        lisp_object_t *func = lt_vector_pop(stack);
        prim = func;
        int arity = primitive_arity(func);
        int restp = primitive_restp(func);
//        Check the number of arguments passed
        if (restp == TRUE) {
          arity--;
          if (nargs < arity)
            return signal_exception("Too few arguments passed");
        } else {
          if (nargs > arity)
            return signal_exception("Too many arguments passed");
          else if (nargs < arity)
            return signal_exception("Too few arguments passed");
        }

        lisp_object_t *val = NULL;
        assert(is_lt_primitive(func));
//        Preprocess the arguments on the stack if the primitive function takes
//        a rest flag.
        if (primitive_restp(func) == TRUE) {
          assert(nargs >= primitive_arity(func) - 1);
          lt *rest = make_empty_list();
          for (int i = nargs - primitive_arity(func) + 1; i > 0; i--) {
            lt *arg = lt_vector_pop(stack);
            rest = make_pair(arg, rest);
          }
          lt_vector_push(stack, rest);
        }
        switch (primitive_arity(func)) {
          case 0:
            val = ((f0)primitive_func(func))();
            break;
          case 1:
            val = ((f1)primitive_func(func))(_arg1);
            break;
          case 2:
            val = ((f2)primitive_func(func))(_arg1, _arg2);
            break;
          case 3:
            val = ((f3)primitive_func(func))(_arg1, _arg2, _arg3);
            break;
          default :
            fprintf(stdout, "Primitive function with arity %d is not supported\n", primitive_arity(func));
            exit(1);
        }
        move_stack();
        lt_vector_push(stack, val);
//        When the primitive function's execution is finished, they will put the return
//        value at the top of stack. If this return value is a signaled exception, and the
//        local variable `throw_exception' is false, it means the last primitive function
//        was called within a `try-with' block. Therefore, the exception object, as a
//        return value, should be left at the top of stack, as the return value, and it
//        will be used by the expandsion code of `try-with' block, in other word, CATCH
//        by the language.
      }
        break;
      case RESTARGS: {
        lt *rest = the_empty_list;
        while (nargs > fixnum_value(op_restargs_count(ins))) {
          lt *arg = lt_vector_pop(stack);
          rest = make_pair(arg, rest);
          nargs--;
        }
        lt_vector_push(stack, rest);
      }
        break;
      case RETURN: {
        if (isnull(return_stack))
          break;
        lisp_object_t *retaddr = pair_head(return_stack);
        if (debug)
          writef(standard_out, "retaddr is %?\n", retaddr);

        return_stack = pair_tail(return_stack);
        code = retaddr_code(retaddr);
        env = retaddr_env(retaddr);
        pc = retaddr_pc(retaddr);
        throw_exception = retaddr_throw_flag(retaddr);
      }
        break;
      case ADDI:
        arg2 = lt_vector_pop(stack);
        arg1 = lt_vector_pop(stack);
        lt_vector_push(stack, lt_fx_add(arg1, arg2));
        break;
      case CONS:
        arg2 = lt_vector_pop(stack);
        arg1 = lt_vector_pop(stack);
        lt_vector_push(stack, make_pair(arg1, arg2));
        break;
      case DIVI:
        arg2 = lt_vector_pop(stack);
        arg1 = lt_vector_pop(stack);
        lt_vector_push(stack, lt_fx_div(arg1, arg2));
        break;
      case MULI:
        arg2 = lt_vector_pop(stack);
        arg1 = lt_vector_pop(stack);
        lt_vector_push(stack, lt_fx_mul(arg1, arg2));
        break;
      case SUBI:
        arg2 = lt_vector_pop(stack);
        arg1 = lt_vector_pop(stack);
        lt_vector_push(stack, lt_fx_sub(arg1, arg2));
        break;
      default :
        fprintf(stdout, "In run_by_llam --- Invalid opcode %d\n", type_of(ins));
        exit(1);
    }
    if (debug)
      writef(standard_out, "stack is %?\n", stack);
    pc++;
  }
  halt:
  assert(isfalse(lt_is_vector_empty(stack)));
  return vlast(stack, 0);
}
