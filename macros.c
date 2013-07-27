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

lt *lt_let_macro(lt *bindings, lt *body) {
  lt *decls = make_empty_list();
  lt *sets = make_empty_list();
  while (ispair(bindings)) {
    lt *bd = pair_head(bindings);
    lt *com;
    com = list2(S("declare"), pair_head(bd));
    decls = make_pair(com, decls);
    com = list3(S("set!"), pair_head(bd), second(bd));
    sets = make_pair(com, sets);
    bindings = pair_tail(bindings);
  }
  decls = lt_list_nreverse(decls);
  sets = lt_list_nreverse(sets);
  return make_pair(S("begin"), seq(decls, sets, body));
}

lt *lt_push_macro(lt *x, lt *list) {
  return list3(S("set!"), list, list3(S("cons"), x, list));
}

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

void init_macros(void) {
  lt *func;
#define DM(arity, restp, func_name, Lisp_name) \
  do { \
    func = make_macro(make_primitive(arity, func_name, Lisp_name, restp), null_env); \
    symbol_value(S(Lisp_name)) = func; \
  } while(0)

  DM(1, TRUE, lt_cond_macro, "cond");
  DM(2, TRUE, lt_let_macro, "let");
  DM(2, FALSE, lt_push_macro, "push");
  DM(1, FALSE, quasiq, "quasiquote");
}
