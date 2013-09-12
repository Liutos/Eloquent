/*
 * utilities.c
 *
 * Defines the utility functions only depends on the operations provided by object.c and type.h
 *
 *  Created on: 2013年7月20日
 *      Author: liutos
 */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <gc/gc.h>

#include "object.h"
#include "type.h"

lt *booleanize(int value) {
  if (value == 0)
    return the_false;
  else
    return the_true;
}

int is_label(lt *object) {
  return issymbol(object);
}

int is_symbol_bound(lt *symbol) {
  return isundef(symbol_value(symbol))? FALSE: TRUE;
}

int is_macro_form(lt *form) {
  if (!ispair(form))
    return FALSE;
  if (!issymbol(pair_head(form)))
    return FALSE;
  lt *symbol = pair_head(form);
  if (isfunction(symbol_macro(symbol)) || isprimitive(symbol_macro(symbol)))
    return TRUE;
  return FALSE;
}

int is_tag_list(lisp_object_t *object, lisp_object_t *tag) {
  return ispair(object) && (pair_head(object) == tag);
}

#define deform_pred(func_name, symbol_name) \
  int func_name(lt *form) { \
    return is_tag_list(form, S(symbol_name)); \
  }

deform_pred(is_begin_form, "begin")
deform_pred(is_catch_form, "catch")
deform_pred(is_goto_form, "goto")
deform_pred(is_if_form, "if")
deform_pred(is_lambda_form, "lambda")
deform_pred(is_quote_form, "quote")
deform_pred(is_set_form, "set!")
deform_pred(is_tagbody_form, "tagbody")
deform_pred(is_var_form, "var")

lt *list1(lt *element) {
  return make_pair(element, make_empty_list());
}

lt *list2(lt *e1, lt *e2) {
  return make_pair(e1, list1(e2));
}

lt *list3(lt *e1, lt *e2, lt *e3) {
  return make_pair(e1, list2(e2, e3));
}

lt *list4(lt *e1, lt *e2, lt *e3, lt *e4) {
  return make_pair(e1, list3(e2, e3, e4));
}

lt *raw_list(lt *e0, ...) {
  lt *lt_list_nreverse(lt *);
  va_list ap;
  va_start(ap, e0);
  e0 = list1(e0);
  lt *next = va_arg(ap, lt *);
  while (next != NULL) {
    e0 = make_pair(next, e0);
    next = va_arg(ap, lt *);
  }
  return lt_list_nreverse(e0);
}

lt *append2(lt *l1, lt *l2) {
  if (isnull(l1))
    return l2;
  else
    return make_pair(pair_head(l1), append2(pair_tail(l1), l2));
}

lisp_object_t *append_n(lisp_object_t *list0, ...) {
  va_list ap;
  va_start(ap, list0);
  lisp_object_t *next = va_arg(ap, lisp_object_t *);
  while (next != NULL) {
    list0 = append2(list0, next);
    next = va_arg(ap, lisp_object_t *);
  }
  return list0;
}

int pair_length(lisp_object_t *pair) {
  if (isnull(pair))
    return 0;
  int length = 0;
  while (!isnull(pair)) {
    assert(ispair(pair));
    length++;
    pair = pair_tail(pair);
  }
  return length;
}

lisp_object_t *reader_error(char *format, ...) {
  static char msg[1000];
  va_list ap;
  va_start(ap, format);
  vsprintf(msg, format, ap);
  return make_exception(strdup(msg), TRUE, S("READER-ERROR"), the_empty_list);
}

char *sb2string(string_builder_t *sb) {
  sb->string[sb->index] = '\0';
  return sb->string;
}

void sb_add_char(string_builder_t *sb, char c) {
  if (sb->index >= sb->length) {
    sb->length += 20;
    sb->string = GC_realloc(sb->string, sb->length * sizeof(char));
  }
  sb->string[sb->index] = c;
  sb->index++;
}

lt *signal_exception(char *message) {
  return make_exception(message, TRUE, S("ERROR"), the_empty_list);
}

lt *signal_typerr(char *type_name) {
  char msg[256];
  sprintf(msg, "Argument is not of type %s", type_name);
  return signal_exception(strdup(msg));
}
