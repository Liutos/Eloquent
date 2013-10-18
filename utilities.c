/*
 * utilities.c
 *
 *  Created on: 2013年7月20日
 *      Author: liutos
 *
 * This file contains definition of all utilities
 */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <gc/gc.h>

#include "object.h"
#include "type.h"
#include "utilities.h"

#define MASK 0x8000

lt *booleanize(int value) {
  if (value == 0)
    return the_false;
  else
    return the_true;
}

int is_label(lt *object) {
  return is_lt_symbol(object);
}

int is_symbol_bound(lt *symbol) {
  return isundef(symbol_value(symbol))? FALSE: TRUE;
}

int is_macro_form(lt *form) {
  if (!is_lt_pair(form))
    return FALSE;
  if (!is_lt_symbol(pair_head(form)))
    return FALSE;
  lt *symbol = pair_head(form);
  if (is_lt_function(symbol_macro(symbol)) || is_lt_primitive(symbol_macro(symbol)))
    return TRUE;
  return FALSE;
}

int is_tag_list(lisp_object_t *object, lisp_object_t *tag) {
  return is_lt_pair(object) && (pair_head(object) == tag);
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
deform_pred(is_let_form, "let")
deform_pred(is_quote_form, "quote")
deform_pred(is_set_form, "set!")
deform_pred(is_tagbody_form, "tagbody")

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
    assert(is_lt_pair(pair));
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
  return make_exception(strdup(msg), TRUE, the_reader_error_symbol, the_empty_list);
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
  return make_exception(message, TRUE, the_error_symbol, the_empty_list);
}

lt *signal_typerr(char *type_name) {
  char msg[256];
  sprintf(msg, "Argument is not of type %s", type_name);
  return signal_exception(strdup(msg));
}

/* Structure */
int compute_field_offset(char *field_name, char *st_name) {
  lt *fs = search_structure(st_name);
  int i = 0;
  while (is_lt_pair(fs)) {
    lt *field = pair_head(fs);
    if (strcmp(symbol_name(field), field_name) == 0)
      return i;
    fs = pair_head(fs);
    i++;
  }
  return -1;
}

/* Special Forms */
lt *let_bindings(lt *form) {
  return pair_head(pair_tail(form));
}

lt *let_body(lt *form) {
  return pair_tail(pair_tail(form));
}

lt *let_vals(lt *bindings) {
  lt *lt_list_nreverse(lt *);
  lt *vals = the_empty_list;
  while (!isnull(bindings)) {
    lt *binding = pair_head(bindings);
    vals = make_pair(pair_head(pair_tail(binding)), vals);
    bindings = pair_tail(bindings);
  }
  vals = lt_list_nreverse(vals);
  return vals;
}

lt *let_vars(lt *bindings) {
  lt *lt_list_nreverse(lt *);
  lt *vars = the_empty_list;
  while (!isnull(bindings)) {
    lt *binding = pair_head(bindings);
    vars = make_pair(pair_head(binding), vars);
    bindings = pair_tail(bindings);
  }
  vars = lt_list_nreverse(vars);
  return vars;
}

/* UTF-8 */
int raw_count1(char byte) {
  int count = 0;
  while ((byte & MASK) == MASK) {
    count++;
    byte = byte << 1;
  }
  return count;
}

int count1(char byte) {
  int tmp = raw_count1(byte);
  return tmp == 0? 1: tmp;
}

int get_low_bits(char byte, int n) {
  int mask = 0;
  for (int i = 0; i < n; i++) {
    mask = (mask << 1) | 1;
  }
  return byte & mask;
}

uint32_t get_code_point(char *str) {
  int n = raw_count1(*str);
  switch (n) {
    case 0:
      return *str;
    case 2:
      return (get_low_bits(str[0], 5) << 6) | get_low_bits(str[1], 6);
    case 3:
      return (get_low_bits(str[0], 4) << 12) | (get_low_bits(str[1], 6) << 6) | get_low_bits(str[2], 6);
    case 4:
      return (get_low_bits(str[0], 3) << 18) | (get_low_bits(str[1], 6) << 12) |
          (get_low_bits(str[2], 6) << 6) | get_low_bits(str[3], 6);
    default :
      return -1;
  }
}

// TODO: Pre-allocate the space for storing ASCII character instead of allocating everytime
lt *make_unicode_char(char c) {
  char *data = GC_MALLOC(1 * sizeof(char));
  data[0] = c;
  return make_unicode(data);
}

/* String */
int C_string_count(char *str) {
  int count = 0;
  int i = 0;
  while (str[i] != '\0') {
    i += count1(str[i]);
    count++;
  }
  return count;
}

lt *wrap_C_string(char *C_str) {
  return make_string(C_string_count(C_str), C_str);
}
