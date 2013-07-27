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

#include "object.h"
#include "type.h"
#include "utilities.h"

void write_object(lt *, lt *);
lt *compile_to_bytecode(lt *);
lt *read_object(lt *);
lt *run_by_llam(lt *);

/* Writer */
void write_raw_char(char c, lt *dest_port) {
  FILE *fp = output_file_file(dest_port);
  fputc(c, fp);
  if (c == '\n') {
    output_file_linum(dest_port)++;
    output_file_colnum(dest_port) = 0;
  } else
    output_file_colnum(dest_port)++;
}

void write_n_spaces(int n, lt *dest) {
  for (int i = 0; i < n; i++)
    write_raw_char(' ', dest);
}

void write_raw_string(char *string, lt *dest_port) {
  while (*string != '\0') {
    write_raw_char(*string, dest_port);
    string++;
  }
}

void writef(lt *dest, const char *format, ...) {
	int nch = 0;
  va_list ap;
  lisp_object_t *arg;

  va_start(ap, format);
  char c = *format;
  while (c != '\0') {
    if (c != '%')
      write_raw_char(c, dest);
    else {
      format++;
      c = *format;
      arg = va_arg(ap, lisp_object_t *);
      switch (c) {
        case 'c':
          assert(ischar(arg));
          write_raw_char(character_value(arg), dest);
          break;
        case 's':
          assert(isstring(arg));
          write_raw_string(string_value(arg), dest);
          break;
        case 'p':
          nch = fprintf(output_file_file(dest), "%p", arg);
          output_file_colnum(dest) += nch;
          break;
        case 'f':
          assert(isfloat(arg));
          nch = fprintf(output_file_file(dest), "%f", float_value(arg));
          output_file_colnum(dest) += nch;
          break;
        case 'd':
          assert(isfixnum(arg));
          nch = fprintf(output_file_file(dest), "%d", fixnum_value(arg));
          output_file_colnum(dest) += nch;
          break;
        case '?':
          write_object(arg, dest);
          break;
        case 'S':
          assert(issymbol(arg));
          write_object(arg, dest);
          break;
        case '%':
          write_raw_char('%', dest);
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
    case ARGS: 
    	writef(dest, "#<ARGS %d>", op_args_arity(opcode)); 
    	break;
    case ARGSD:
      writef(dest, "#<ARGSD %d>", op_argsd_arity(opcode));
      break;
    case CALL: 
    	writef(dest, "#<CALL %d>", op_call_arity(opcode)); 
    	break;
    case CATCH:
      write_raw_string("#<CATCH>", dest);
      break;
    case CONST: 
    	writef(dest, "#<CONST %?>", op_const_value(opcode)); 
    	break;
    case FJUMP: 
    	writef(dest, "#<FJUMP %?>", op_fjump_label(opcode)); 
    	break;
    case FN: 
    	writef(dest, "#<FN %?>", op_fn_func(opcode)); 
    	break;
    case GSET: 
    	writef(dest, "#<GSET %S>", op_gset_var(opcode)); 
    	break;
    case GVAR: 
    	writef(dest, "#<GVAR %S>", op_gvar_var(opcode)); 
    	break;
    case JUMP: 
    	writef(dest, "#<JUMP %?>", op_jump_label(opcode)); 
    	break;
    case LSET:
      writef(dest, "#<LSET %d %d %S>",
             op_lset_i(opcode), op_lset_j(opcode), op_lset_var(opcode));
      break;
    case LVAR:
      writef(dest, "#<LVAR %d %d %S>",
             op_lvar_i(opcode), op_lvar_j(opcode), op_lvar_var(opcode));
      break;
    case MACROFN:
    	writef(dest, "#<MACRO_FN %?>", op_macro_func(opcode));
    	break;
    case POP: 
    	write_raw_string("#<POP>", dest); 
    	break;
    case PRIM: 
    	writef(dest, "#<PRIM %d>", op_prim_nargs(opcode)); 
    	break;
    case RETURN: 
    	write_raw_string("#<RETURN>", dest); 
    	break;
    default :
      printf("Unknown opcode\n");
      exit(1);
  }
}

void write_compiled_function(lt *function, int indent, lt *dest) {
  lt *lt_type_of(lt *);
	writef(dest, "#<COMPILED-FUNCTION %p\n", function);
	if (!isvector(function_code(function))) {
	  writef(standard_out, "type_of(function_code(function)) is %S\n", lt_type_of(function_code(function)));
	  assert(isvector(function_code(function)));
	}
	for (int i = 0; i < vector_length(function_code(function)); i++) {
		lt *ins = vector_value(function_code(function))[i];
		write_n_spaces(indent, dest);
		if (is_label(ins)) {
			writef(dest, "%S:", ins);
			int rest_width = 8 - (strlen(symbol_name(ins)) + 2);
			write_n_spaces(rest_width, dest);
			continue;
		} else
			write_raw_string("    ", dest);
		write_raw_string(opcode_op(ins), dest);
		int rest_width = 8 - strlen(opcode_op(ins));
		write_n_spaces(rest_width, dest);
		if (opcode_name(ins) == FN) {
			write_compiled_function(op_fn_func(ins), output_file_colnum(dest), dest);
		} else {
			for (int j = 0; j < vector_length(opcode_oprands(ins)); j++) {
				write_object(vector_value(opcode_oprands(ins))[j], dest);
				if (j != vector_length(opcode_oprands(ins)) - 1)
					write_raw_char(' ', dest);
			}
		}
		write_raw_char('\n', dest);
	}
	for (int j = 0; j < indent; j++)
		write_raw_char(' ', dest);
	write_raw_char('>', dest);
}

void write_object(lt *x, lt *output_file) {
  assert(x != NULL);
  switch(type_of(x)) {
    case BOOL:
      if (is_true_object(x))
        write_raw_string("#t", output_file);
      else
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
    	int indent = output_file_colnum(output_file);
    	write_compiled_function(x, indent, output_file);
    }
      break;
    case EMPTY_LIST:
    	write_raw_string("()", output_file);
    	break;
    case EXCEPTION:
      write_raw_string("ERROR: ", output_file);
      write_raw_string(exception_msg(x), output_file);
      break;
    case FIXNUM:
    	writef(output_file, "%d", x);
    	break;
    case FLOAT:
    	writef(output_file, "%f", x);
    	break;
    case INPUT_FILE:
    	writef(output_file, "#<INPUT-FILE %p>");
    	break;
    case MACRO:
    	writef(output_file, "#<MACRO %?>", macro_procedure(x));
      break;
    case OUTPUT_FILE:
    	writef(output_file, "#<OUTPUT-FILE %p>", x);
    	break;
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
    case SYMBOL:
    	write_raw_string(symbol_name(x), output_file);
    	break;
    case TEOF:
    	write_raw_string("#<EOF>", output_file);
    	break;
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

int get_char(lt *input_file) {
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

lt *lt_raw_nthtail(lt *list, int n) {
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

/* Function */
lt *lt_eval(lt *form) {
  return run_by_llam(compile_to_bytecode(form));
}

lt *lt_simple_apply(lt *function, lt *args) {
  assert(isprimitive(function) || isfunction(function));
  assert(ispair(args) || isnull(args));
  lt *expr = make_pair(function, args);
  writef(standard_out, "In lt_simple_apply --- expr is %?\n", expr);
  return run_by_llam(compile_to_bytecode(expr));
}

lt *compress_args(lt *args, int nrequired) {
  lt *lt_list_nreverse(lt *);
  lt *new_args = make_empty_list();
  for (int i = 0; i < nrequired; i++) {
    new_args = make_pair(pair_head(args), new_args);
    args = pair_tail(args);
  }
  lt *rest = make_empty_list();
  while (ispair(args)) {
    rest = make_pair(pair_head(args), rest);
    args = pair_tail(args);
  }
  new_args = make_pair(lt_list_nreverse(rest), new_args);
  return lt_list_nreverse(new_args);
}

lt *lt_expand_macro(lt *form) {
  if (is_macro_form(form)) {
    lt *op = symbol_value(pair_head(form));
    lt *proc = macro_procedure(op);
    assert(isprimitive(proc) || isfunction(proc));
    lt *result;
//      TODO: Combine the two cases of function type
    if (isprimitive(proc)) {
      lt *args = pair_tail(form);
      if (primitive_restp(proc))
        args = compress_args(args, primitive_arity(proc) - 1);
      switch (primitive_arity(proc)) {
        case 0:
          result = ((f0) primitive_func(proc))();
          break;
        case 1: {
          lt *arg1 = lt_raw_nth(args, 0);
          result = ((f1) primitive_func(proc))(arg1);
        }
          break;
        case 2: {
          lt *arg1 = lt_raw_nth(args, 0);
          lt *arg2 = lt_raw_nth(args, 1);
          result = ((f2) primitive_func(proc))(arg1, arg2);
        }
          break;
        default:
          printf("Macro with arity %d is unsupported yet.\n",
          primitive_arity(proc));
          exit(1);
      }
    } else {
      lt *args = pair_tail(form);
      result = lt_simple_apply(proc, args);
    }
    return lt_expand_macro(result);
  } else
      return form;
}

lt *lt_function_arity(lt *function) {
  lt *lt_list_length(lt *);
  assert(isprimitive(function) || isfunction(function));
  if (isprimitive(function))
    return make_fixnum(primitive_arity(function));
  else
    return lt_list_length(function_args(function));
}

lt *lt_load(lt *path) {
  lt *lt_close_in(lt *);
  lt *lt_open_in(lt *);
  assert(isstring(path));
  lt *file = lt_open_in(path);
  lt *expr = read_object(file);
  while (!iseof(expr)) {
    lt_eval(expr);
    expr = read_object(file);
  }
  lt_close_in(file);
  return make_true();
}

/* Input Port */
lt *lt_close_in(lt *file) {
  assert(isinput_file(file));
  fclose(input_file_file(file));
  input_file_openp(file) = TRUE;
  return make_true();
}

lt *lt_is_file_open(lt *file) {
  assert(isinput_file(file) || isoutput_file(file));
  if (isinput_file(file))
    return booleanize(input_file_openp(file));
  else
    return booleanize(output_file_openp(file));
}

lt *lt_open_in(lt *path) {
  assert(isstring(path));
  FILE *fp = fopen(string_value(path), "r");
  return make_input_file(fp);
}

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
lt *lt_list_length(lt *list) {
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
    return the_empty_list;
  if (isnull(pair_tail(list)))
    return list;
  lt *rhead = the_empty_list;
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
    return the_empty_list;
  if (isnull(pair_tail(list)))
    return list;
  else
    return append2(lt_list_reverse(pair_tail(list)), list1(pair_head(list)));
}

/* Arithmetic Operations */
/* TODO: Find a more elegant way of defining arithmetic operations. */
lt *lt_add(lt *n, lt *m) {
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

/* Output File */
lt *lt_close_out(lt *file) {
  assert(isoutput_file(file));
  fclose(output_file_file(file));
  output_file_openp(file) = FALSE;
  return make_true();
}

lt *lt_open_out(lt *path) {
  assert(isstring(path));
  FILE *fp = fopen(string_value(path), "w");
  return make_output_file(fp);
}

lt *lt_write_char(lt *c, lt *dest) {
  write_raw_char(character_value(c), dest);
  return c;
}

lt *lt_write_string(lt *str, lt *dest) {
  write_raw_string(string_value(str), dest);
  return str;
}

lt *lt_write_line(lt *str, lt *dest) {
  lt_write_string(str, dest);
  write_raw_char('\n', dest);
  return str;
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

lt *lt_string_set(lt *string, lt *index, lt *new_char) {
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

lt *lt_is_bound(lt *symbol) {
  assert(issymbol(symbol));
  return booleanize(!isundef(symbol_value(symbol)));
}

lt *lt_is_fbound(lt *symbol) {
  assert(issymbol(symbol));
  if (isundef(symbol_value(symbol)))
    return make_false();
  lt *value = symbol_value(symbol);
  if (isprimitive(value) || isfunction(value))
    return make_true();
  else
    return make_false();
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
  if (v1 == v2) return the_true;
  if (vector_length(v1) != vector_length(v2))
    return the_false;
  for (int i = 0; i < vector_length(v1); i++) {
    if (isfalse(lt_equal(vector_value(v1)[i], vector_value(v2)[i])))
      return the_false;
  }
  return the_true;
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

lt *lt_vector_set(lt *vector, lt *index, lt *new_value) {
  if (!isvector(vector))
    return signal_typerr("VECTOR");
  if (!isfixnum(index))
    return signal_typerr("FIXNUM");
  if (!(vector_length(vector) > fixnum_value(index)))
    return signal_exception("The second argument is too large to index.");
  vector_value(vector)[fixnum_value(index)] = new_value;
  return vector;
}

lt *lt_vector_to_list(lt *vector) {
  lt *lt_list_nreverse(lt *);
  int length = vector_length(vector);
  lt *list = make_empty_list();
  for (int i = 0; i < length; i++) {
    list = make_pair(vector_value(vector)[i], list);
  }
  return lt_list_nreverse(list);
}

/* List */
lt *lt_append(lt *lists) {
  if (isnull(lists))
    return make_empty_list();
  return append2(pair_head(lists), lt_append(pair_tail(lists)));
}

lisp_object_t *lt_head(lisp_object_t *pair) {
  assert(ispair(pair));
  return pair_head(pair);
}

lt *lt_is_tag_list(lt *list, lt *tag) {
  return booleanize(is_tag_list(list, tag));
}

lt *lt_list(lt *list) {
  return list;
}

lt *lt_list_equal(lt *l1, lt *l2) {
  lt *lt_equal(lt *, lt *);
  if (l1 == l2)
    return the_true;
  while (!isnull(l1) && !isnull(l2)) {
    if (isfalse(lt_equal(pair_head(l1), pair_head(l2))))
      return the_false;
    l1 = pair_tail(l1);
    l2 = pair_tail(l2);
  }
  if (!isnull(l1) || !isnull(l2))
    return the_false;
  else
    return the_true;
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
    return the_true;
  if (isnumber(x) && isnumber(y))
    return lt_numeric_eq(x, y);
  if (ischar(x) && ischar(y))
    return booleanize(character_value(x) == character_value(y));
  return the_false;
}

lt *lt_equal(lt *x, lt *y) {
  if (!isfalse(lt_eql(x, y)))
    return the_true;
  if (ispair(x) && ispair(y))
    return lt_list_equal(x, y);
  if (isvector(x) && isvector(y))
    return lt_vector_equal(x, y);
  return the_false;
}

lt *lt_is_constant(lt *object) {
  if (is_tag_list(object, S("quote")))
    return make_true();
  if (!ispair(object) && !issymbol(object))
    return make_true();
  return make_false();
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
    mktype(VECTOR);
    mktype(MACRO);
    mktype(PAIR);
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
  return the_empty_list;
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
    return the_empty_list;
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
            return the_true;
          else
          	goto bool_error_label;
        case 'f':
          if (isdelimiter(peek_char(input_file)))
            return the_false;
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
        return the_empty_list;
      lisp_object_t *tail = read_pair(input_file);
      if (is_signaled(tail))
        return tail;
      else
        return make_pair(head, tail);
    }
    case ']': case ')':
    	return make_close();
    case '.':
    	return the_dot_symbol;
    case '[':
    	return read_vector(input_file);
    case '\'':
      return list2(S("quote"), read_object(input_file));
    case '`':
      return list2(S("quasiquote"), read_object(input_file));
    case ',': {
      c = get_char(input_file);
      if (c == '@')
        return list2(S("unquote-splicing"), read_object(input_file));
      unget_char(c, input_file);
      return list2(S("unquote"), read_object(input_file));
    }
      break;
    default :
    read_symbol_label:
      return read_symbol(c, input_file);
  }
}

lisp_object_t *read_object_from_string(char *text) {
  FILE *in = fmemopen(text, strlen(text), "r");
  lisp_object_t *inf = make_input_file(in);
  lt *obj = read_object(inf);
  fclose(in);
  return obj;
}

lt *lt_read_from_string(lt *string) {
  return read_object_from_string(string_value(string));
}

void init_prims(void) {
#define ADD(arity, restp, function_name, Lisp_name)                            \
  do {                                                                  \
    func =                                                              \
        make_primitive(arity, (void *)function_name, Lisp_name, restp);        \
    symbol_value(S(Lisp_name)) = func;                                  \
  } while (0)

  lisp_object_t *func;
  /* Arithmetic operations */
  ADD(2, FALSE, lt_add, "+");
  ADD(2, FALSE, lt_div, "/");
  ADD(2, FALSE, lt_gt, ">");
  ADD(2, FALSE, lt_mod, "mod");
  ADD(2, FALSE, lt_mul, "*");
  ADD(2, FALSE, lt_numeric_eq, "=");
  ADD(2, FALSE, lt_sub, "-");
  /* Character */
  ADD(1, FALSE, lt_char_code, "char-code");
  ADD(1, FALSE, lt_code_char, "code-char");
  /* Function */
  ADD(1, FALSE, lt_eval, "eval");
  ADD(1, FALSE, lt_expand_macro, "expand-macro");
  ADD(1, FALSE, lt_function_arity, "function-arity");
  ADD(1, FALSE, lt_load, "load");
  ADD(2, FALSE, lt_simple_apply, "simple-apply");
  /* Input File */
  ADD(1, FALSE, lt_close_in, "close-in");
  ADD(1, FALSE, lt_is_file_open, "file-open?");
  ADD(1, FALSE, lt_open_in, "open-in");
  ADD(1, FALSE, lt_read_char, "read-char");
  ADD(1, FALSE, lt_read_line, "read-line");
  ADD(1, FALSE, lt_read_from_string, "read-from-string");
  /* List */
  ADD(1, TRUE, lt_append, "append");
  ADD(2, FALSE, lt_is_tag_list, "is-tag-list?");
  ADD(2, FALSE, make_pair, "cons");
  ADD(1, FALSE, lt_head, "head");
  ADD(1, TRUE, lt_list, "list");
  ADD(1, FALSE, lt_list_length, "list-length");
  ADD(1, FALSE, lt_list_nreverse, "list-reverse!");
  ADD(1, FALSE, lt_list_reverse, "list-reverse");
  ADD(2, FALSE, lt_nth, "nth");
  ADD(2, FALSE, lt_nthtail, "nth-tail");
  ADD(2, FALSE, lt_set_head, "set-head");
  ADD(2, FALSE, lt_set_tail, "set-tail");
  ADD(1, FALSE, lt_tail, "tail");
  /* Output File */
  ADD(1, FALSE, lt_open_in, "open-in");
  ADD(1, FALSE, lt_open_out, "open-out");
  ADD(2, FALSE, lt_write_char, "write-char");
  ADD(2, FALSE, lt_write_line, "write-line");
  ADD(2, FALSE, lt_write_string, "write-string");
  /* String */
  ADD(2, FALSE, lt_char_at, "char-at");
  ADD(1, FALSE, lt_string_length, "string-length");
  ADD(3, FALSE, lt_string_set, "string-set");
  /* Symbol */
  ADD(1, FALSE, lt_intern, "string->symbol");
  ADD(1, FALSE, lt_is_bound, "bound?");
  ADD(1, FALSE, lt_is_fbound, "fbound?");
  ADD(1, FALSE, lt_symbol_name, "symbol-name");
  ADD(1, FALSE, lt_symbol_value, "symbol-value");
  /* Vector */
  ADD(1, FALSE, lt_list_to_vector, "list->vector");
  ADD(2, FALSE, lt_vector_ref, "vector-ref");
  ADD(3, FALSE, lt_vector_set, "vector-set!");
  ADD(1, FALSE, lt_vector_to_list, "vector->list");
  /* General */
  ADD(1, FALSE, lt_is_constant, "is-constant?");
  ADD(2, FALSE, lt_eq, "eq");
  ADD(2, FALSE, lt_eql, "eql");
  ADD(2, FALSE, lt_equal, "equal");
  ADD(0, FALSE, lt_object_size, "object-size");
  ADD(1, FALSE, lt_type_of, "type-of");
}
