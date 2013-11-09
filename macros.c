/*
 * macros.c
 *
 *  Created on: 2013年7月26日
 *      Author: liutos
 *
 * This file contains the definition of all predefined macros
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
  return is_tag_list(clause, LISP("else"));
}

lt *seq_to_exp(lt *seq) {
  if (isnull(seq))
    return make_empty_list();
  else if (isnull(pair_tail(seq)))
    return pair_head(seq);
  else
    return make_pair(the_begin_symbol, seq);
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
          make_pair(the_if_symbol,
              list3(cond_pred(first),
                    seq_to_exp(cond_actions(first)),
                    lt_cond_macro(rest)));
    }
  }
}

/* pset! */
lt *get_ks(lt *kvs) {
  if (isnull(kvs))
    return the_empty_list;
  else
    return make_pair(pair_head(kvs), get_ks(pair_tail(pair_tail(kvs))));
}

lt *get_vs(lt *kvs) {
  if (isnull(kvs))
    return the_empty_list;
  else
    return make_pair(pair_head(pair_tail(kvs)), get_vs(pair_tail(pair_tail(kvs))));
}

lt *ngensym(int n) {
  if (n == 0)
    return the_empty_list;
  else
    return make_pair(lt_gensym(), ngensym(n - 1));
}

lt *lt_pset_macro(lt *kvs) {
  lt *ks = get_ks(kvs);
  lt *vs = get_vs(kvs);
  lt *ns = ngensym(pair_length(vs));
  lt *bd = the_empty_list;
  lt *saved = ns;
  while (!isnull(vs)) {
    lt *expr = pair_head(vs);
    lt *name = pair_head(ns);
    bd = make_pair(list2(name, expr), bd);
    vs = pair_tail(vs);
    ns = pair_tail(ns);
  }
  bd = list_nreverse(bd);
  ns = saved;
  lt *sets = the_empty_list;
  while (!isnull(ks)) {
    lt *var = pair_head(ks);
    lt *name = pair_head(ns);
    sets = make_pair(list3(the_set_symbol, var, name), sets);
    ks = pair_tail(ks);
    ns = pair_tail(ns);
  }
  sets = list_nreverse(sets);
  lt *result = make_pair(LISP("let"), make_pair(bd, sets));
  return result;
}

/* quasiquote */
lt *quasiq(lt *x) {
  if (!is_lt_pair(x)) {
    if (!isfalse(lt_is_constant(x)))
      return x;
    else
      return list2(the_quote_symbol, x);
  }
  if (is_tag_list(x, the_unquote_symbol)) {
    assert(!isnull(pair_tail(x)));
    assert(isnull(pair_tail(pair_tail(x))));
    return second(x);
  }
  if (is_tag_list(x, the_quasiquote_symbol)) {
    assert(!isnull(pair_tail(x)));
    assert(isnull(pair_tail(pair_tail(x))));
    return quasiq(quasiq(second(x)));
  }
  if (is_tag_list(pair_head(x), the_splicing_symbol)) {
    if (isnull(pair_tail(x)))
      return second(pair_head(x));
    else
      return
          list3(LISP("append"),
                second(pair_head(x)),
                quasiq(pair_tail(x)));
  }
  if (is_lt_pair(x))
    return list3(LISP("cons"), quasiq(pair_head(x)), quasiq(pair_tail(x)));
  writef(standard_out, "Unknown case of quasiquote %?\n", x);
  exit(1);
}

/* try-catch */
lt *handler_tag(lt *handler) {
  return list2(the_quote_symbol, pair_head(handler));
}

lt *handler_var(lt *handler) {
  return pair_head(second(handler));
}

lt *handler_action(lt *handler) {
  return lt_raw_nthtail(handler, 2);
}

lt *make_single_let(lt *var, lt *val, lt *action) {
  lt *bindings = list1(list2(var, val));
  return make_pair(LISP("let"), make_pair(bindings, action));
}

lt *handler2if(lt *ex, lt *handlers) {
  if (isnull(handlers))
    return ex;
  else {
    lt *handler = pair_head(handlers);
    lt *rest = pair_tail(handlers);
    lt *pred = list3(LISP("eql?"), list2(LISP("exception-tag"), ex), handler_tag(handler));
    lt *tp = make_single_let(handler_var(handler), ex, handler_action(handler));
    return make_pair(LISP("if"), list3(pred, tp, handler2if(ex, rest)));
  }
}

lt *try_catch_macro(lt *form, lt *handlers) {
  lt *tmp = lt_gensym();
  lt *handler_forms = handler2if(tmp, handlers);
  lt *bd = list2(tmp, form);
  lt *catch = list1(the_catch_symbol);
  lt *lf = make_pair(LISP("let"), list2(list1(bd), handler_forms));
  return list3(the_begin_symbol, catch, lf);
}

void init_macros(void) {
  lt *func;
#define DM(arity, restp, func_name, Lisp_name) \
  do { \
    func = make_primitive(arity, func_name, Lisp_name, restp); \
    symbol_macro(LISP(Lisp_name)) = func; \
  } while(0)

  DM(1, TRUE, lt_cond_macro, "cond");
  DM(2, TRUE, try_catch_macro, "try-catch");
  DM(1, TRUE, lt_pset_macro, "pset!");
  DM(1, FALSE, quasiq, "quasiquote");
}
