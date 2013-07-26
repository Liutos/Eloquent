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
    quasiq(quasiq(second(x)));
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

  DM(2, FALSE, lt_push_macro, "push");
  DM(1, FALSE, quasiq, "quasiquote");
}
