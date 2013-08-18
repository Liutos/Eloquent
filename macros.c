/*
 * macros.c
 *
 *  Created on: 2013年7月26日
 *      Author: liutos
 */
#include <assert.h>
#include <stdlib.h>

#include "object.h"
#include "prims.h"
#include "type.h"
#include "utilities.h"

/* cond */
lt *cond_pred(lt *clause) {
  return pair_head(clause);
}

lt *cond_actions(lt *clause) {
  return pair_tail(clause);
}

int is_else_clause(lt *clause) {
  return is_tag_list(clause, S("else"));
}

lt *seq_to_exp(lt *seq) {
  if (isnull(seq))
    return make_empty_list();
  else if (isnull(pair_tail(seq)))
    return pair_head(seq);
  else
    return make_pair(S("begin"), seq);
}

lt *lt_cond_macro(lt *clauses) {
  if (isnull(clauses))
    return make_false();
  else {
    lt *first = pair_head(clauses);
    lt *rest = pair_tail(clauses);
    if (is_else_clause(first)) {
      if (isnull(rest))
        return seq_to_exp(cond_actions(first));
      else {
        printf("ELSE clause isn't last -- lt_cond_macro");
        exit(1);
      }
    } else {
      return
          make_pair(S("if"),
              list3(cond_pred(first),
                    seq_to_exp(cond_actions(first)),
                    lt_cond_macro(rest)));
    }
  }
}

/* let */
lt *lt_let_macro(lt *bindings, lt *body) {
  lt *pars= make_empty_list();
  lt *args= make_empty_list();
  if (isnull(bindings))
    return make_pair(S("begin"), body);
  while (ispair(bindings)) {
    lt *bd = pair_head(bindings);
    pars = make_pair(pair_head(bd), pars);
    args = make_pair(second(bd), args);
    bindings = pair_tail(bindings);
  }
  pars = lt_list_nreverse(pars);
  args = lt_list_nreverse(args);
  lt *lambda = make_pair(S("lambda"), make_pair(pars, body));
  return make_pair(lambda, args);
}

/* var */
lt *lt_var_macro(lt *var, lt *val) {
  lt *decl = list2(S("declare"), var);
  lt *setf = list3(S("set!"), var, val);
  return list3(S("begin"), decl, setf);
}

/* push */
lt *lt_push_macro(lt *x, lt *list) {
  return list3(S("set!"), list, list3(S("cons"), x, list));
}

/* quasiquote */
lt *quasiq(lt *x) {
  if (!ispair(x)) {
    if (!isfalse(lt_is_constant(x)))
      return x;
    else
      return list2(S("quote"), x);
  }
  if (is_tag_list(x, S("unquote"))) {
    assert(!isnull(pair_tail(x)));
    assert(isnull(pair_tail(pair_tail(x))));
    return second(x);
  }
  if (is_tag_list(x, S("quasiquote"))) {
    assert(!isnull(pair_tail(x)));
    assert(isnull(pair_tail(pair_tail(x))));
    return quasiq(quasiq(second(x)));
  }
  if (is_tag_list(pair_head(x), S("unquote-splicing"))) {
    if (isnull(pair_tail(x)))
      return second(pair_head(x));
    else
      return
          list3(S("append"),
                second(pair_head(x)),
                quasiq(pair_tail(x)));
  }
  if (ispair(x))
    return list3(S("cons"), quasiq(pair_head(x)), quasiq(pair_tail(x)));
  writef(standard_out, "Unknown case of quasiquote %?\n", x);
  exit(1);
}

/* try-catch */
lt *handler_tag(lt *handler) {
  return list2(S("quote"), pair_head(handler));
}

lt *handler_var(lt *handler) {
  return pair_head(second(handler));
}

lt *handler_action(lt *handler) {
  return lt_nthtail(handler, make_fixnum(2));
}

lt *make_single_let(lt *var, lt *val, lt *action) {
  lt *bindings = list1(list2(var, val));
  return make_pair(S("let"), make_pair(bindings, action));
}

lt *handler2if(lt *ex, lt *handlers) {
  if (isnull(handlers))
    return ex;
  else {
    lt *handler = pair_head(handlers);
    lt *rest = pair_tail(handlers);
    lt *pred = list3(S("eql?"), list2(S("exception-tag"), ex), handler_tag(handler));
    lt *tp = make_single_let(handler_var(handler), ex, handler_action(handler));
    return make_pair(S("if"), list3(pred, tp, handler2if(ex, rest)));
  }
}

lt *try_catch_macro(lt *form, lt *handlers) {
  lt *tmp = lt_gensym();
  lt *handler_forms = handler2if(tmp, handlers);
  lt *val = list3(S("var"), tmp, form);
  lt *catch = list1(S("catch"));
  return list4(S("begin"), catch, val, handler_forms);
}

/* name-lambda */
lt *tco(lt *name, lt *pars, lt *form, int islast) {
  if (!ispair(form))
    return form;
  if (is_macro_form(form))
    return tco(name, pars, lt_expand_macro(form), islast);
  lt *op = pair_head(form);
  if (op == S("if")) {
    lt *pred = second(form);
    lt *tp = tco(name, pars, third(form), islast);
    lt *ep = tco(name, pars, fourth(form), islast);
    return list4(S("if"), pred, tp, ep);
  }
  if (op == S("begin")) {
    lt *actions = pair_tail(form);
    lt *sets = make_empty_list();
    if (!(isnull(actions) || isnull(pair_tail(actions)))) {
      while (!isnull(pair_tail(actions))) {
        lt *p = pair_head(actions);
        sets = make_pair(tco(name, pars, p, FALSE), sets);
        actions = pair_tail(actions);
      }
    }
    return make_pair(S("begin"),
        seq(sets, list1(tco(name, pars, pair_head(actions), islast))));
  }
  if (op == name && islast) {
    lt *tmp = pair_tail(form);
    lt *args = make_empty_list();
    while (ispair(tmp)) {
      lt *par = pair_head(pars);
      lt *arg = pair_head(tmp);
      arg = list3(S("set!"), par, arg);
      args = make_pair(arg, args);
      pars = pair_tail(pars);
      tmp = pair_tail(tmp);
    }
    args = lt_list_nreverse(args);
    return make_pair(S("begin"),
        seq(args, list1(list2(S("goto"), name))));
  } else {
    lt *tmp = pair_tail(form);
    lt *args = make_empty_list();
    while (ispair(tmp)) {
      lt *arg = pair_head(tmp);
      arg = tco(name, pars, arg, FALSE);
      args = make_pair(arg, args);
      tmp = pair_tail(tmp);
    }
    args = lt_list_nreverse(args);
    return make_pair(op, args);
  }
}

lt *name_lambda_macro(lt *name, lt *pars, lt *body) {
  lt *expr = tco(name, pars, make_pair(S("begin"), body), TRUE);
  expr = make_pair(S("tagbody"), make_pair(name, pair_tail(expr)));
  expr = list3(S("lambda"), pars, expr);
  return expr;
}

void init_macros(void) {
  lt *func;
#define DM(arity, restp, func_name, Lisp_name) \
  do { \
    func = make_macro(make_primitive(arity, func_name, Lisp_name, restp), null_env); \
    symbol_value(S(Lisp_name)) = func; \
  } while(0)

  DM(1, TRUE, lt_cond_macro, "cond");
  DM(2, TRUE, lt_let_macro, "let");
  DM(3, TRUE, name_lambda_macro, "name-lambda");
  DM(2, TRUE, try_catch_macro, "try-catch");
  DM(2, FALSE, lt_var_macro, "var");
  DM(2, FALSE, lt_push_macro, "push");
  DM(1, FALSE, quasiq, "quasiquote");
}
