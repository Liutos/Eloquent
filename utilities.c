/*
 * utilities.c
 *
 *  Created on: 2013年7月20日
 *      Author: liutos
 */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "type.h"

lisp_object_t *booleanize(int value) {
  if (value == 0)
    return the_false;
  else
    return the_true;
}

int is_symbol_bound(lisp_object_t *symbol) {
  return isundef(symbol_value(symbol))? FALSE: TRUE;
}

lisp_object_t *list1(lisp_object_t *o) {
  return make_pair(o, make_empty_list());
}

lt *signal_exception(char *msg) {
  return make_exception(msg, TRUE);
}

lt *signal_typerr(char *type_name) {
  char msg[256];
  sprintf(msg, "Argument is not of type %s", type_name);
  return signal_exception(strdup(msg));
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
  return make_exception(strdup(msg), TRUE);
}

string_builder_t *make_str_builder(void) {
  string_builder_t *sb = malloc(sizeof(*sb));
  sb->length = 20;
  sb->string = malloc(sb->length * sizeof(char));
  sb->index = 0;
  return sb;
}

void sb_add_char(string_builder_t *sb, char c) {
  if (sb->index >= sb->length) {
    sb->length += 20;
    sb->string = realloc(sb->string, sb->length * sizeof(char));
  }
  sb->string[sb->index] = c;
  sb->index++;
}

char *sb2string(string_builder_t *sb) {
  sb->string[sb->index] = '\0';
  return sb->string;
}

