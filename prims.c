/*
 * prims.c
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

lt *read_object(lt *);
void write_object(lt *, lt *);

/* PART: prims.c */
/* DONE: Moves the following section of code to above. */
/* Utilities */
lisp_object_t *booleanize(int value) {
  if (value == 0)
    return false;
  else
    return true;
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

/* Writer */
void write_raw_char(char c, lisp_object_t *out_port) {
  FILE *fp = output_file_file(out_port);
  fputc(c, fp);
  if (c == '\n') {
    output_file_linum(out_port)++;
    output_file_colnum(out_port) = 0;
  } else
    output_file_colnum(out_port)++;
}

void write_raw_string(char *string, lisp_object_t *output_file) {
  while (*string != '\0') {
    write_raw_char(*string, output_file);
    string++;
  }
}

void writef(lisp_object_t *out_port, const char *format, ...) {
  va_list ap;
  char c;
  lisp_object_t *arg;
  int nch;

  va_start(ap, format);
  c = *format;
  while (c != '\0') {
    if (c != '%')
      write_raw_char(c, out_port);
    else {
      format++;
      c = *format;
      arg = va_arg(ap, lisp_object_t *);
      switch (c) {
        case 'c':
          assert(ischar(arg));
          write_raw_char(character_value(arg), out_port);
          break;
        case 's':
          assert(isstring(arg));
          write_raw_string(string_value(arg), out_port);
          break;
        case 'p':
          nch = fprintf(output_file_file(out_port), "%p", arg);
          output_file_colnum(out_port) += nch;
          break;
        case 'f':
          assert(isfloat(arg));
          nch = fprintf(output_file_file(out_port), "%f", float_value(arg));
          output_file_colnum(out_port) += nch;
          break;
        case 'd':
          assert(isfixnum(arg));
          nch = fprintf(output_file_file(out_port), "%d", fixnum_value(arg));
          output_file_colnum(out_port) += nch;
          break;
        case '?':
          write_object(arg, out_port);
          break;
        case 'S':
          assert(issymbol(arg));
          write_object(arg, out_port);
          break;
        case '%':
          write_raw_char('%', out_port);
          break;
        default :
          fprintf(stdout, "Invalid character %c after %%", c);
          exit(1);
      }
    }
    format++;
    c = *format;
  }
}

void write_opcode(lt *opcode, lt *dest) {
  switch (opcode_type(opcode)) {
    case ARGS: writef(dest, "#<ARGS %d>", op_args_arity(opcode)); break;
    case CALL: writef(dest, "#<CALL %d>", op_call_arity(opcode)); break;
    case CATCH:
      writef(dest, "#<CATCH type_name: %S, handler: %?>",
             op_catch_type(opcode), op_catch_handler(opcode));
      break;
    case CONST: writef(dest, "#<CONST %?>", op_const_value(opcode)); break;
    case FJUMP: writef(dest, "#<FJUMP %?>", op_fjump_label(opcode)); break;
    case FN: writef(dest, "#<FN %?>", op_fn_func(opcode)); break;
    case GSET: writef(dest, "#<GSET %S>", op_gset_var(opcode)); break;
    case GVAR: writef(dest, "#<GVAR %S>", op_gvar_var(opcode)); break;
    case JUMP: writef(dest, "#<JUMP %?>", op_jump_label(opcode)); break;
    case LSET:
      writef(dest, "#<LSET %d %d %S>",
             op_lset_i(opcode), op_lset_j(opcode), op_lset_var(opcode));
      break;
    case LVAR:
      writef(dest, "#<LVAR %d %d %S>",
             op_lvar_i(opcode), op_lvar_j(opcode), op_lvar_var(opcode));
      break;
    case POP: write_raw_string("#<POP>", dest); break;
    case PRIM: writef(dest, "#<PRIM %d>", op_prim_nargs(opcode)); break;
    case RETURN: write_raw_string("#<RETURN>", dest); break;
    default :
      printf("Unknown opcode\n");
      exit(1);
  }
}

void write_object(lisp_object_t *x, lisp_object_t *output_file) {
  if (x == NULL) {
    fprintf(stdout, "Impossible!!! The code has errors!!!\n");
    exit(1);
  }
  switch(type_of(x)) {
    case BOOL:
      if (is_true_object(x))
        write_raw_string("#t", output_file);
      if (isfalse(x))
        write_raw_string("#f", output_file);
      break;
    case CHARACTER: {
      int c = character_value(x);
      switch (c) {
        case ' ':
          write_raw_string("#\\space", output_file);
          break;
        case '\n':
          write_raw_string("#\\newline", output_file);
          break;
        default :
          writef(output_file, "#\\%c", x);
      }
    }
      break;
    case FUNCTION: {
      writef(output_file, "#<COMPILED-FUNCTION %p\n", x);
      lisp_object_t *code_vector = function_code(x);
      assert(isvector(code_vector));
      for (int i = 0; i < vector_length(code_vector); i++) {
        writef(output_file, "\t%?", vector_value(code_vector)[i]);
        if (i < vector_length(code_vector) - 1)
          write_raw_char('\n', output_file);
      }
      write_raw_string(">", output_file);
    }
      break;
    case EMPTY_LIST: write_raw_string("()", output_file); break;
    case EXCEPTION:
      write_raw_string("ERROR: ", output_file);
      write_raw_string(exception_msg(x), output_file);
      break;
    case FIXNUM: writef(output_file, "%d", x); break;
    case FLOAT: writef(output_file, "%f", x); break;
    case INPUT_FILE: writef(output_file, "#<INPUT-FILE %p>"); break;
    case OUTPUT_FILE: writef(output_file, "#<OUTPUT-FILE %p>", x); break;
    case PAIR:
      write_raw_string("(", output_file);
      write_object(pair_head(x), output_file);
      for (x = pair_tail(x); ispair(x); x = pair_tail(x)) {
        write_raw_string(" ", output_file);
        write_object(pair_head(x), output_file);
      }
      if (!isnull(x)) {
        write_raw_string(" . ", output_file);
        write_object(x, output_file);
      }
      write_raw_string(")", output_file);
      break;
    case PRIMITIVE_FUNCTION:
      write_raw_string("#<PRIMITIVE-FUNCTION ", output_file);
      write_raw_string(primitive_Lisp_name(x), output_file);
      writef(output_file, " %p>", x);
      break;
    case RETADDR:
      writef(output_file, "#<RETADDR %p pc: %d>", x, make_fixnum(retaddr_pc(x)));
      break;
    case STRING: {
      char *value = string_value(x);
      write_raw_string("\"", output_file);
      for (int i = 0; value[i] != '\0'; i++) {
        if (value[i] == '"')
          write_raw_string("\\\"", output_file);
        else
          write_raw_char(value[i], output_file);
      }
      write_raw_string("\"", output_file);
    }
      break;
    case SYMBOL: write_raw_string(symbol_name(x), output_file); break;
    case TEOF: write_raw_string("#<EOF>", output_file); break;
    case VECTOR: {
      lisp_object_t **vector = vector_value(x);
      write_raw_string("[", output_file);
      for (int i = 0; i <= vector_last(x); i++) {
        write_object(vector[i], output_file);
        if (i != vector_last(x))
          write_raw_string(" ", output_file);
      }
      write_raw_string("]", output_file);
    }
      break;
    case OPCODE: write_opcode(x, output_file); break;
    default :
      fprintf(stdout, "invalid object with type %d", type_of(x));
      exit(1);
  }
}

void write_expr(char *expr, lisp_object_t *result) {
  writef(standard_out, "%s => %?\n", make_string(expr), result);
}

/* Primitives */
/* Auxiliary Functions */
int get_char(lisp_object_t *input_file) {
  FILE *in = input_file_file(input_file);
  input_file_colnum(input_file)++;
  return getc(in);
}

lt *lt_raw_nth(lt *list, int n) {
  assert(ispair(list));
  for (int i = 0; i < n; i++) {
    list = pair_tail(list);
    if (!ispair(list)) {
      char msg[256];
      sprintf(msg, "This list is too short for indexing %d", n);
      return signal_exception(strdup(msg));
    }
  }
  return pair_head(list);
}

lisp_object_t *lt_raw_nthtail(lisp_object_t *list, int n) {
  assert(ispair(list));
  int n2 = n;
  while (n2 > 0) {
    if (!ispair(list)) {
      fprintf(stdout, "This list is too short for index %d\n", n);
      exit(1);
    }
    list = pair_tail(list);
    n2--;
  }
  return list;
}

lisp_object_t *lt_append2(lisp_object_t *l1, lisp_object_t *l2) {
  if (isnull(l1))
    return l2;
  else
    return make_pair(pair_head(l1), lt_append2(pair_tail(l1), l2));
}

/* Input Port */
lt *lt_read_char(lt *in_port) {
  assert(isinput_file(in_port));
  return make_character(get_char(in_port));
}

lt *lt_read_line(lt *in_port) {
  assert(isinput_file(in_port));
  string_builder_t *sb = make_str_builder();
  int c = get_char(in_port);
  while (c != EOF && c != '\n') {
    sb_add_char(sb, c);
    c = get_char(in_port);
  }
  return make_string(sb2string(sb));
}

/* List */
lisp_object_t *lt_list_length(lisp_object_t *list) {
  if (isnull(list))
    return make_fixnum(0);
  int length = 0;
  while (!isnull(list)) {
    if (!ispair(list))
      return signal_exception("Argument is not a proper list.");
    length++;
    list = pair_tail(list);
  }
  return make_fixnum(length);
}

lt *lt_list_nreverse(lt *list) {
  if (isnull(list))
    return null_list;
  if (isnull(pair_tail(list)))
    return list;
  lt *rhead = null_list;
  lt *rest = list;
  while (!isnull(rest)) {
    if (!ispair(rest))
      return signal_exception("Argument is not a proper list.");
    lt *tmp = pair_tail(rest);
    pair_tail(rest) = rhead;
    rhead = rest;
    rest = tmp;
  }
  return rhead;
}

lt *lt_list_reverse(lt *list) {
  if (isnull(list))
    return null_list;
  if (isnull(pair_tail(list)))
    return list;
  else
    return lt_append2(lt_list_reverse(pair_tail(list)), list1(pair_head(list)));
}

/* Arithmetic operations */
int get_numeric_level(lt *n) {
  switch (type_of(n)) {
    case FIXNUM: return 0;
    case FLOAT: return 1;
    default :
      fprintf(stdout, "In get_numeric_level --- It's impossible!\n");
      exit(1);
  }
}

int is_lower_than(lt *n1, lt *n2) {
  return get_numeric_level(n1) < get_numeric_level(n2);
}

/* TODO: Find a more elegant way of defining arithmetic operations. */
lisp_object_t *lt_add(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return make_fixnum(fixnum_value(n) + fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return make_float(fixnum_value(n) + float_value(m));
  if (isfloat(n) && isfixnum(m))
    return make_float(float_value(n) + fixnum_value(m));
  else
    return make_float(float_value(n) + float_value(m));
}

lisp_object_t *lt_div(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return make_fixnum(fixnum_value(n) / fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return make_float(fixnum_value(n) / float_value(m));
  if (isfloat(n) && isfixnum(m))
    return make_float(float_value(n) / fixnum_value(m));
  else
    return make_float(float_value(n) / float_value(m));
}

lisp_object_t *lt_gt(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return booleanize(fixnum_value(n) > fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return booleanize(fixnum_value(n) > float_value(m));
  if (isfloat(n) && isfixnum(m))
    return booleanize(float_value(n) > fixnum_value(m));
  else
    return booleanize(float_value(n) > float_value(m));
}

lisp_object_t *lt_mod(lisp_object_t *n, lisp_object_t *m) {
  assert(isfixnum(n) && isfixnum(m));
  return make_fixnum(fixnum_value(n) % fixnum_value(m));
}

lisp_object_t *lt_mul(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return make_fixnum(fixnum_value(n) * fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return make_float(fixnum_value(n) * float_value(m));
  if (isfloat(n) && isfixnum(m))
    return make_float(float_value(n) * fixnum_value(m));
  else
    return make_float(float_value(n) * float_value(m));
}

lisp_object_t *lt_numeric_eq(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return booleanize(fixnum_value(n) == fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return booleanize(fixnum_value(n) == float_value(m));
  if (isfloat(n) && isfixnum(m))
    return booleanize(float_value(n) == fixnum_value(m));
  else
    return booleanize(float_value(n) == float_value(m));
}

lisp_object_t *lt_sub(lisp_object_t *n, lisp_object_t *m) {
  assert(isnumber(n) && isnumber(m));
  if (isfixnum(n) && isfixnum(m))
    return make_fixnum(fixnum_value(n) - fixnum_value(m));
  if (isfixnum(n) && isfloat(m))
    return make_float(fixnum_value(n) - float_value(m));
  if (isfloat(n) && isfixnum(m))
    return make_float(float_value(n) - fixnum_value(m));
  else
    return make_float(float_value(n) - float_value(m));
}

/* Character */
lisp_object_t *lt_char_code(lisp_object_t *c) {
  assert(ischar(c));
  return make_fixnum(character_value(c));
}

lisp_object_t *lt_code_char(lisp_object_t *code) {
  assert(isfixnum(code));
  return make_character(fixnum_value(code));
}

/* String */
lisp_object_t *lt_char_at(lisp_object_t *string, lisp_object_t *index) {
  assert(isstring(string) && isfixnum(index));
  assert(strlen(string_value(string)) > fixnum_value(index));
  return make_character(string_value(string)[fixnum_value(index)]);
}

lisp_object_t *lt_string_length(lisp_object_t *string) {
  assert(isstring(string));
  return make_fixnum(strlen(string_value(string)));
}

lisp_object_t *lt_string_set(lisp_object_t *string, lisp_object_t *index, lisp_object_t *new_char) {
  assert(isstring(string));
  assert(isfixnum(index));
  assert(ischar(new_char));
  string_value(string)[fixnum_value(index)] = character_value(new_char);
  return string;
}

/* Symbol */
lisp_object_t *lt_intern(lisp_object_t *name) {
  assert(isstring(name));
  return find_or_create_symbol(string_value(name));
}

lisp_object_t *lt_symbol_name(lisp_object_t *symbol) {
  assert(issymbol(symbol));
  return make_string(strdup(symbol_name(symbol)));
}

lisp_object_t *lt_symbol_value(lisp_object_t *symbol) {
  assert(issymbol(symbol));
  return symbol_value(symbol);
}

/* Vector */
lisp_object_t *lt_is_vector_empty(lisp_object_t *vector) {
  assert(isvector(vector));
  return booleanize(vector_last(vector) <= -1);
}

lisp_object_t *lt_is_vector_full(lisp_object_t *vector) {
  assert(isvector(vector));
  return booleanize(vector_last(vector) >= vector_length(vector) - 1);
}

lisp_object_t *lt_list_to_vector(lisp_object_t *list) {
  assert(ispair(list));
  int len = pair_length(list);
  lisp_object_t *vector = make_vector(len);
  for (int i = 0; i < len; i++) {
    vector_value(vector)[i] = pair_head(list);
    vector_last(vector)++;
    list = pair_tail(list);
  }
  return vector;
}

lt *lt_vector_equal(lt *v1, lt *v2) {
  lt *lt_equal(lt *, lt *);
  if (v1 == v2) return true;
  if (vector_length(v1) != vector_length(v2))
    return false;
  for (int i = 0; i < vector_length(v1); i++) {
    if (isfalse(lt_equal(vector_value(v1)[i], vector_value(v2)[i])))
      return false;
  }
  return true;
}

lisp_object_t *lt_vector_last_nth(lisp_object_t *vector, lisp_object_t *n) {
  assert(isvector(vector) && isfixnum(n));
  assert(isfalse(lt_is_vector_empty(vector)));
  assert(vector_last(vector) >= fixnum_value(n));
  int index = vector_last(vector) - fixnum_value(n);
  return vector_value(vector)[index];
}

lisp_object_t *lt_vector_pop(lisp_object_t *vector) {
  assert(isvector(vector));
  if (!isfalse(lt_is_vector_empty(vector))) {
    fprintf(stdout, "The vector is empty\n");
    exit(1);
  }
  vector_last(vector)--;
  return vector_value(vector)[vector_last(vector) + 1];
}

lisp_object_t *lt_vector_push(lisp_object_t *vector, lisp_object_t *object) {
  assert(isvector(vector));
  if (!isfalse(lt_is_vector_full(vector))) {
    fprintf(stdout, "The vector is full\n");
    exit(1);
  }
  vector_last(vector)++;
  vector_value(vector)[vector_last(vector)] = object;
  return vector;
}

lisp_object_t *lt_vector_ref(lisp_object_t *vector, lisp_object_t *index) {
  assert(isvector(vector));
  assert(isfixnum(index));
  assert(vector_last(vector) > fixnum_value(index));
  return vector_value(vector)[fixnum_value(index)];
}

lisp_object_t *lt_vector_set(lisp_object_t *vector, lisp_object_t *index, lisp_object_t *new_value) {
  if (!isvector(vector))
    return signal_typerr("VECTOR");
  if (!isfixnum(index))
    return signal_typerr("FIXNUM");
  if (!(vector_length(vector) > fixnum_value(index)))
    return signal_exception("The second argument is too large to index.");
  vector_value(vector)[fixnum_value(index)] = new_value;
  return vector;
}

/* List */
lisp_object_t *lt_append(lisp_object_t *list0, ...) {
  va_list ap;
  va_start(ap, list0);
  lisp_object_t *next = va_arg(ap, lisp_object_t *);
  while (next != NULL) {
    list0 = lt_append2(list0, next);
    next = va_arg(ap, lisp_object_t *);
  }
  return list0;
}

lisp_object_t *lt_head(lisp_object_t *pair) {
  assert(ispair(pair));
  return pair_head(pair);
}

lt *lt_list_equal(lt *l1, lt *l2) {
  lt *lt_equal(lt *, lt *);
  if (l1 == l2)
    return true;
  while (!isnull(l1) && !isnull(l2)) {
    if (isfalse(lt_equal(pair_head(l1), pair_head(l2))))
      return false;
    l1 = pair_tail(l1);
    l2 = pair_tail(l2);
  }
  if (!isnull(l1) || !isnull(l2))
    return false;
  else
    return true;
}

lisp_object_t *lt_nth(lisp_object_t *list, lisp_object_t *n) {
  if (!ispair(list))
    return signal_exception("The first argument is not a pair.");
  if (!isfixnum(n))
    return signal_exception("The second argument is not a fixnum.");
  int n2 = fixnum_value(n);
  return lt_raw_nth(list, n2);
}

lisp_object_t *lt_nthtail(lisp_object_t *list, lisp_object_t *n) {
  assert(ispair(list) && isfixnum(n));
  int n2 = fixnum_value(n);
  return lt_raw_nthtail(list, n2);
}

lisp_object_t *lt_set_head(lisp_object_t *pair, lisp_object_t *new_head) {
  assert(ispair(pair));
  pair_head(pair) = new_head;
  return pair;
}

lisp_object_t *lt_set_tail(lisp_object_t *pair, lisp_object_t *new_tail) {
  assert(ispair(pair));
  pair_tail(pair) = new_tail;
  return pair;
}

lisp_object_t *lt_tail(lisp_object_t *pair) {
  assert(ispair(pair));
  return pair_tail(pair);
}

/* General */
lisp_object_t *lt_eq(lisp_object_t *x, lisp_object_t *y) {
  return booleanize(x == y);
}

lt *lt_eql(lt *x, lt *y) {
  if (x == y)
    return true;
  if (isnumber(x) && isnumber(y))
    return lt_numeric_eq(x, y);
  if (ischar(x) && ischar(y))
    return booleanize(character_value(x) == character_value(y));
  return false;
}

lt *lt_equal(lt *x, lt *y) {
  if (!isfalse(lt_eql(x, y)))
    return true;
  if (ispair(x) && ispair(y))
    return lt_list_equal(x, y);
  if (isvector(x) && isvector(y))
    return lt_vector_equal(x, y);
  return false;
}

lt *lt_object_size(void) {
  return make_fixnum(sizeof(lt));
}

lisp_object_t *lt_type_of(lisp_object_t *object) {
#define mktype(type) case type: return S(#type)

  switch (type_of(object)) {
    mktype(BOOL);
    mktype(CHARACTER);
    mktype(FIXNUM);
    mktype(FLOAT);
    mktype(FUNCTION);
    mktype(PRIMITIVE_FUNCTION);
    default :
      fprintf(stdout, "Unknown type %d of object\n", type_of(object));
      exit(1);
  }
}

/* Reader */
int peek_char(lisp_object_t *input_file) {
  FILE *in = input_file_file(input_file);
  int c = getc(in);
  ungetc(c, in);
  return c;
}

void unget_char(int c, lisp_object_t *input_file) {
  ungetc(c, input_file_file(input_file));
  input_file_colnum(input_file)--;
}

int isdelimiter(int c) {
  int ds[] = { EOF, ' ', '\n', '(', ')', '"', '[', ']', ';' };
  for (int i = 0; i < sizeof(ds) / sizeof(int); i++) {
    if (ds[i] == c)
      return TRUE;
  }
  return FALSE;
}

lt *expect_string(char *target, lisp_object_t *input_file) {
  while (*target != '\0') {
    int c = get_char(input_file);
    if (c != *target) {
      return reader_error("Unexpected character '%c'. Expecting '%c'", c, *target);
    }
    target++;
  }
  return null_list;
}

lisp_object_t *read_character(lisp_object_t *input_file) {
  int c = get_char(input_file);
  lt *tmp;
  switch (c) {
    case 's':
      c = peek_char(input_file);
      if (isdelimiter(c))
        return make_character('s');
      tmp = expect_string("pace", input_file);
      if (is_signaled(tmp))
        return tmp;
      return make_character(' ');
    case 'n':
      c = peek_char(input_file);
      if (isdelimiter(c))
        return make_character('n');
      tmp = expect_string("ewline", input_file);
      if (is_signaled(tmp))
        return tmp;
      return make_character('\n');
    default : {
      int c2 = peek_char(input_file);
      if (isdelimiter(c2))
        return make_character(c);
      else {
        fprintf(stdout, "Unexpected character '%c' after '%c'\n", c2, c);
        exit(1);
      }
    }
  }
}

lisp_object_t *read_float(lisp_object_t *input_file, int integer) {
  int e = 1;
  int sum = 0;
  int c = get_char(input_file);
  for (; isdigit(c); c = get_char(input_file)) {
    e *= 10;
    sum = sum * 10 + c - '0';
  }
  unget_char(c, input_file);
  return make_float(integer + sum * 1.0 / e);
}

lisp_object_t *read_fixnum(lisp_object_t *input_file, int sign, char start) {
  int sum = start - '0';
  int c = get_char(input_file);
  for (; isdigit(c); c = get_char(input_file)) {
    sum = sum * 10 + c - '0';
  }
  if (c == '.')
    return read_float(input_file, sum);
  else
    unget_char(c, input_file);
  return make_fixnum(sign * sum);
}

lisp_object_t *read_pair(lisp_object_t *input_file) {
  lisp_object_t *obj = read_object(input_file);
  if (iseof(obj))
    return reader_error("Unexpected end-of-file.");
  if (is_signaled(obj))
    return obj;
  if (isclose(obj))
    return null_list;
  if (isdot(obj)) {
    lisp_object_t *o1 = read_object(input_file);
    if (is_signaled(o1))
      return o1;
    lisp_object_t *o2 = read_object(input_file);
    if (isclose(o1))
      return reader_error("Too few tokens after dot");
    else if (isclose(o2))
      return o1;
    else
      return reader_error("multiple tokens in dotted tail");
  } else {
    lisp_object_t *tail = read_pair(input_file);
    if (is_signaled(tail))
      return tail;
    else
      return make_pair(obj, tail);
  }
}

lisp_object_t *read_string(lisp_object_t *input_file) {
  string_builder_t *buffer = make_str_builder();
  for (;;) {
    int c = get_char(input_file);
    if (c == '"')
      return make_string(sb2string(buffer));
    if (c == '\\') {
      c = get_char(input_file);
      switch (c) { case 'n': c = '\n'; break; case 't': c = '\t'; break;}
    }
    if (c == EOF)
      return reader_error("Reading string. Unexpected end-of-file.");
    sb_add_char(buffer, c);
  }
  return reader_error("The string is too long");
}

lisp_object_t *read_symbol(char start, lisp_object_t *input_file) {
  string_builder_t *buffer = make_str_builder();
  sb_add_char(buffer, start);
  int c = get_char(input_file);
  int i = 1;
  for (; !isdelimiter(c); i++) {
    sb_add_char(buffer, c);
    c = get_char(input_file);
  }
  if (isdelimiter(c) && c != EOF)
    unget_char(c, input_file);
  return find_or_create_symbol(sb2string(buffer));
}

lisp_object_t *read_vector(lisp_object_t *input_file) {
  lisp_object_t *list = read_pair(input_file);
  return lt_list_to_vector(list);
}

lisp_object_t *read_object(lisp_object_t *input_file) {
  int c = get_char(input_file);
  switch (c) {
    case ';':
      while ((c = get_char(input_file)) != EOF && c != '\n');
      return read_object(input_file);
    case EOF:
    	return make_eof();
    case '\n': case '\r': case '\t':
    	input_file_linum(input_file)++;
    	return read_object(input_file);
    case ' ':
    	return read_object(input_file);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return read_fixnum(input_file, 1, c);
    case '-':
      if (isdigit(peek_char(input_file)))
        return read_fixnum(input_file, -1, '0');
      else
        goto read_symbol_label;
    case '#':
      c = get_char(input_file);
      switch (c) {
        case '\\':
        	return read_character(input_file);
        case 't':
          if (isdelimiter(peek_char(input_file)))
            return true;
          else
          	goto bool_error_label;
        case 'f':
          if (isdelimiter(peek_char(input_file)))
            return false;
          else
          	goto bool_error_label;
        default : {
        	bool_error_label:
          return reader_error("Unexpected character '%c' after '#', at line %d, column %d", c, input_file_linum(input_file), input_file_colnum(input_file));
        }
      }
      break;
    case '"':
    	return read_string(input_file);
    case '(': {
      lisp_object_t *head = read_object(input_file);
      if (isclose(head))
        return null_list;
      lisp_object_t *tail = read_pair(input_file);
      if (is_signaled(tail))
        return tail;
      else
        return make_pair(head, tail);
    }
    case ']': case ')':
    	return make_close();
    case '.':
    	return dot_symbol;
    case '[':
    	return read_vector(input_file);
    case '\'':
      return make_pair(S("quote"), list1(read_object(input_file)));
    default :
    read_symbol_label:
      return read_symbol(c, input_file);
  }
}

lisp_object_t *read_object_from_string(char *text) {
  FILE *in = fmemopen(text, strlen(text), "r");
  lisp_object_t *inf = make_input_file(in);
  return read_object(inf);
}

void init_prims(void) {
#define ADD(arity, function_name, Lisp_name)                            \
  do {                                                                  \
    func =                                                              \
        make_primitive(arity, (void *)function_name, Lisp_name);        \
    symbol_value(S(Lisp_name)) = func;                                  \
  } while (0)

  lisp_object_t *func;
  /* Arithmetic operations */
  ADD(2, lt_add, "+");
  ADD(2, lt_div, "/");
  ADD(2, lt_gt, ">");
  ADD(2, lt_mod, "mod");
  ADD(2, lt_mul, "*");
  ADD(2, lt_numeric_eq, "=");
  ADD(2, lt_sub, "-");
  /* Character */
  ADD(1, lt_char_code, "char-code");
  ADD(1, lt_code_char, "code-char");
  /* Input File */
  ADD(1, lt_read_char, "read-char");
  ADD(1, lt_read_line, "read-line");
  /* List */
  ADD(1, lt_head, "head");
  ADD(1, lt_list_length, "list-length");
  ADD(1, lt_list_nreverse, "list-reverse!");
  ADD(1, lt_list_reverse, "list-reverse");
  ADD(2, lt_nth, "nth");
  ADD(2, lt_nthtail, "nth-tail");
  ADD(2, lt_set_head, "set-head");
  ADD(2, lt_set_tail, "set-tail");
  ADD(1, lt_tail, "tail");
  /* String */
  ADD(2, lt_char_at, "char-at");
  ADD(1, lt_string_length, "string-length");
  ADD(3, lt_string_set, "string-set");
  /* Symbol */
  ADD(1, lt_intern, "string->symbol");
  ADD(1, lt_symbol_name, "symbol-name");
  ADD(1, lt_symbol_value, "symbol-value");
  /* Vector */
  ADD(1, lt_list_to_vector, "list->vector");
  ADD(2, lt_vector_ref, "vector-ref");
  ADD(3, lt_vector_set, "vector-set!");
  /* General */
  ADD(2, lt_eq, "eq");
  ADD(2, lt_eql, "eql");
  ADD(2, lt_equal, "equal");
  ADD(0, lt_object_size, "object-size");
  ADD(1, lt_type_of, "type-of");
}
