/*
 * prototype.c
 *
 *
 *
 * Copyright (C) 2013-06-07 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Collects this part to file typedef.h */
typedef struct lisp_object_t lisp_object_t;
typedef lisp_object_t lt;
typedef lt *(*f0)(void);
typedef lisp_object_t *(*f1)(lisp_object_t *);
typedef lisp_object_t *(*f2)(lisp_object_t *, lisp_object_t *);
typedef lisp_object_t *(*f3)(lisp_object_t *, lisp_object_t *, lisp_object_t *);

lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env);
lisp_object_t *read_object(lisp_object_t *input_file);
void write_object(lisp_object_t *object, lisp_object_t *out_port);
void writef(lt *, const char *, ...);

/* tagging system
 *   bits end in  00:  pointer
 *                01:  fixnum
 *              0110:  char
 *              1110:  other immediate object (null_list, true, false)
 */
#define CHAR_BITS 4
#define CHAR_MASK 15
#define CHAR_TAG 6
#define FIXNUM_BITS 2
#define FIXNUM_MASK 3
#define FIXNUM_TAG 1
#define IMMEDIATE_BITS 4
#define IMMEDIATE_MASK 15
#define IMMEDIATE_TAG 14
#define POINTER_MASK 3
#define POINTER_TAG 0

enum TYPE {
  BOOL,
  /* TODO: The support for Unicode. */
  CHARACTER,
  COMPILED_FUNCTION,
  EMPTY_LIST,
  EXCEPTION,
  /* TODO: Implements the arbitrary precision arithmetic numeric types. */
  /* TODO: Implements a part of types as tagged-pointer. */
  FIXNUM,
  FLOAT,
  INPUT_FILE,
  OPCODE,
  OUTPUT_FILE,
  PAIR,
  PRIMITIVE_FUNCTION,
  RETADDR,
  STRING,
  SYMBOL,
  TCLOSE,
  TEOF,
  VECTOR,
  UNDEF,
};

enum OPCODE_TYPE {
  ARGS,
  CALL,
  CONST,
  FN,
  GSET,
  GVAR,
  FJUMP,
  JUMP,
  LSET,
  LVAR,
  POP,
  PRIM,
  RETURN,
};

struct lisp_object_t {
  int gc_mark_flag;
  int use_flag;
  lt *next;
  enum TYPE type;
  union {
    struct {
      int value;
    } boolean;
    struct {
      char value;
    } character;
    struct {
      char *message;
      int signal_flag;
    } exception;
    struct {
      int value;
    } fixnum;
    struct {
      float value;
    } float_num;
    struct {
      lisp_object_t *code;
      lisp_object_t *env;
      lisp_object_t *args;
    } function;
    struct {
      FILE *file;
      int linum;
      int colnum;
    } input_file;
    struct {
      enum OPCODE_TYPE name;
      lt *oprands;
    } opcode;
    struct {
      FILE *file;
      int linum;
      int colnum;
    } output_file;
    struct {
      lisp_object_t *head;
      lisp_object_t *tail;
    } pair;
    struct {
      int arity;
      void *C_function;
      char *Lisp_name;
    } primitive;
    struct {
      lisp_object_t *code;
      lisp_object_t *env;
      int pc;
    } retaddr;
    struct {
      char *value;
      int length;
    } string;
    struct {
      char *name;
      lisp_object_t *global_value;
    } symbol;
    struct {
      int last;
      int length;
      lisp_object_t **value;
    } vector;
    /* DONE: Defines all the opcodes as a single nested tagged-union. */
  } u;
};

#define FALSE 0
#define TRUE 1
#define OBJECT_INIT_COUNT 1000
#define pub

/* Accessors */
#define boolean_value(x)  ((x)->u.boolean.value)
#define character_value(x) ((x)->u.character.value)
#define exception_msg(x) ((x)->u.exception.message)
#define exception_flag(x) ((x)->u.exception.signal_flag)
/* #define fixnum_value(x) (x->u.fixnum.value) */
#define fixnum_value(x) (((int)(x)) >> FIXNUM_BITS)
#define float_value(x) ((x)->u.float_num.value)
#define function_args(x) ((x)->u.function.args)
#define function_env(x) ((x)->u.function.env)
#define function_code(x) ((x)->u.function.code)
#define input_file_colnum(x) ((x)->u.input_file.colnum)
#define input_file_file(x) ((x)->u.input_file.file)
#define input_file_linum(x) ((x)->u.input_file.linum)
#define opcode_name(x) ((x)->u.opcode.name)
#define opcode_oprands(x) ((x)->u.opcode.oprands)
#define output_file_colnum(x) ((x)->u.output_file.colnum)
#define output_file_file(x) ((x)->u.output_file.file)
#define output_file_linum(x) ((x)->u.output_file.linum)
#define pair_head(x) (x->u.pair.head)
#define pair_tail(x) (x->u.pair.tail)
#define primitive_Lisp_name(x) ((x)->u.primitive.Lisp_name)
#define primitive_arity(x) ((x)->u.primitive.arity)
#define primitive_func(x) ((x)->u.primitive.C_function)
#define retaddr_code(x) ((x)->u.retaddr.code)
#define retaddr_env(x) ((x)->u.retaddr.env)
#define retaddr_pc(x) ((x)->u.retaddr.pc)
#define string_value(x) ((x)->u.string.value)
#define symbol_name(x) ((x)->u.symbol.name)
#define symbol_value(x) ((x)->u.symbol.global_value)
#define vector_last(x) ((x)->u.vector.last)
#define vector_length(x) (x->u.vector.length)
#define vector_value(x) (x->u.vector.value)
/* DONE: Defines the following accessors macro by indexing. */
/* Opcode Accessors */
#define opcode_type(x) opcode_name(x)
#define opargn(x, n) (vector_value(opcode_oprands(x))[n])
#define oparg1(x) opargn(x, 0)
#define oparg2(x) opargn(x, 1)
#define oparg3(x) opargn(x, 2)

#define op_args_arity(x) oparg1(x)
#define op_call_arity(x) oparg1(x)
#define op_const_value(x) oparg1(x)
#define op_fjump_label(x) oparg1(x)
#define op_fn_func(x) oparg1(x)
#define op_gset_var(x) oparg1(x)
#define op_gvar_var(x) oparg1(x)
#define op_jump_label(x) oparg1(x)
#define op_lset_i(x) oparg1(x)
#define op_lset_j(x) oparg2(x)
#define op_lset_var(x) oparg3(x)
#define op_lvar_i(x) oparg1(x)
#define op_lvar_j(x) oparg2(x)
#define op_lvar_var(x) oparg3(x)
#define op_prim_nargs(x) oparg1(x)

#define S(name) (find_or_create_symbol(name))

#define first(x) lt_raw_nth((x), 0)
#define second(x) lt_raw_nth((x), 1)
#define third(x) lt_raw_nth((x), 2)
#define fourth(x) lt_raw_nth((x), 3)

#define seq(...) lt_append(__VA_ARGS__, NULL)

/* Global variables */
int debug_flag;
lisp_object_t *dot_symbol;
lisp_object_t *false;
lisp_object_t *true;
lisp_object_t *null_env;
lisp_object_t *null_list;
lt *object_pool;                        /* An array contains lt. */
lisp_object_t *standard_in;
lisp_object_t *standard_out;
lisp_object_t *symbol_list;
lisp_object_t *undef_object;
lt *free_objects;
lt *used_objects;

/* TODO: Collects this part to file object.c */
/* Constructors */
lt *allocate_object(void) {
  if (free_objects == NULL) {
    printf("Pool is full, program terminated.\n");
    exit(1);
  }
  lt *obj = free_objects;
  free_objects = free_objects->next;
  obj->gc_mark_flag = FALSE;
  obj->use_flag = TRUE;
  return obj;
}

void *checked_malloc(size_t size) {
  void *p = malloc(size);
  assert(p != NULL);
  return p;
}

lisp_object_t *make_object(enum TYPE type) {
  lt *obj = allocate_object();
  obj->type = type;
  return obj;
}

lisp_object_t *make_boolean(int value) {
  lisp_object_t *boolean = make_object(BOOL);
  boolean_value(boolean) = value;
  return boolean;
}

lisp_object_t *make_character(char value) {
  lisp_object_t *character = make_object(CHARACTER);
  character->u.character.value = value;
  return character;
}

lisp_object_t *make_close(void) {
  lisp_object_t *close = make_object(TCLOSE);
  return close;
}

#define mksingle_type(func_name, type)          \
  lisp_object_t *func_name(void) {              \
    lisp_object_t *obj = make_object(type);     \
    return obj;                                 \
  }

mksingle_type(make_empty_list, EMPTY_LIST)
mksingle_type(make_eof, TEOF)
mksingle_type(make_undef, UNDEF)

lt *make_exception(char *message, int signal_flag) {
  lt *ex = make_object(EXCEPTION);
  exception_msg(ex) = message;
  exception_flag(ex) = signal_flag;
  return ex;
}

/* lisp_object_t *make_fixnum(int value) { */
/*   lisp_object_t *fixnum = make_object(FIXNUM); */
/*   fixnum_value(fixnum) = value; */
/*   return fixnum; */
/* } */
lt *make_fixnum(int value) {
  return (lt *)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lisp_object_t *make_float(float value) {
  lisp_object_t *flt_num = make_object(FLOAT);
  float_value(flt_num) = value;
  return flt_num;
}

lisp_object_t *make_function(lisp_object_t *env, lisp_object_t *args, lisp_object_t *code) {
  lisp_object_t *func = make_object(COMPILED_FUNCTION);
  function_env(func) = env;
  function_args(func) = args;
  function_code(func) = code;
  return func;
}

lisp_object_t *make_input_file(FILE *file) {
  lisp_object_t *inf = make_object(INPUT_FILE);
  input_file_file(inf) = file;
  input_file_linum(inf) = 1;
  input_file_colnum(inf) = 0;
  return inf;
}

lisp_object_t *make_output_file(FILE *file) {
  lisp_object_t *outf = make_object(OUTPUT_FILE);
  output_file_file(outf) = file;
  output_file_linum(outf) = 1;
  output_file_colnum(outf) = 0;
  return outf;
}

lisp_object_t *make_pair(lisp_object_t *head, lisp_object_t *tail) {
  lisp_object_t *pair = make_object(PAIR);
  pair_head(pair) = head;
  pair_tail(pair) = tail;
  return pair;
}

lisp_object_t *make_primitive(int arity, void *C_function, char *Lisp_name) {
  lisp_object_t *p = make_object(PRIMITIVE_FUNCTION);
  primitive_arity(p) = arity;
  primitive_func(p) = C_function;
  primitive_Lisp_name(p) = Lisp_name;
  return p;
}

lisp_object_t *make_retaddr(lisp_object_t *code, lisp_object_t *env, int pc) {
  lisp_object_t *retaddr = make_object(RETADDR);
  retaddr_code(retaddr) = code;
  retaddr_env(retaddr) = env;
  retaddr_pc(retaddr) = pc;
  return retaddr;
}

lisp_object_t *make_string(char *value) {
  lisp_object_t *string = make_object(STRING);
  string->u.string.value = value;
  return string;
}

lisp_object_t *make_symbol(char *name) {
  lisp_object_t *symbol = make_object(SYMBOL);
  symbol->u.symbol.name = name;
  symbol_value(symbol) = undef_object;
  return symbol;
}

lisp_object_t *make_vector(int length) {
  lisp_object_t *vector = make_object(VECTOR);
  vector_last(vector) = -1;
  vector_length(vector) = length;
  vector_value(vector) = checked_malloc(length * sizeof(lisp_object_t *));
  return vector;
}

/* Opcode constructors */
lt *make_opcode(enum OPCODE_TYPE name, lt *oprands) {
  lt *obj = make_object(OPCODE);
  opcode_name(obj) = name;
  opcode_oprands(obj) = oprands;
  return obj;
}

lt *mkopcode(enum OPCODE_TYPE name, int arity, ...) {
  lt *oprands = make_vector(arity);
  va_list ap;
  va_start(ap, arity);
  for (int i = 0; i < arity; i++)
    vector_value(oprands)[i] = va_arg(ap, lt *);
  return make_opcode(name, oprands);
}

lisp_object_t *make_op_args(lisp_object_t *length) {
  return mkopcode(ARGS, 1, length);
}

lisp_object_t *make_op_call(lisp_object_t *arity) {
  return mkopcode(CALL, 1, arity);
}

lisp_object_t *make_op_const(lisp_object_t *value) {
  return mkopcode(CONST, 1, value);
}

lisp_object_t *make_op_fjump(lisp_object_t *label) {
  return mkopcode(FJUMP, 1, label);
}

lisp_object_t *make_op_fn(lisp_object_t *func) {
  return mkopcode(FN, 1, func);
}

lisp_object_t *make_op_gset(lisp_object_t *symbol) {
  return mkopcode(GSET, 1, symbol);
}

lisp_object_t *make_op_gvar(lisp_object_t *symbol) {
  return mkopcode(GVAR, 1, symbol);
}

lisp_object_t *make_op_jump(lisp_object_t *label) {
  return mkopcode(JUMP, 1, label);
}

lisp_object_t *make_op_lset(lisp_object_t *i, lisp_object_t *j, lisp_object_t *symbol) {
  return mkopcode(LSET, 3, i, j, symbol);
}

lisp_object_t *make_op_lvar(lisp_object_t *i, lisp_object_t *j, lisp_object_t *symbol) {
  return mkopcode(LVAR, 3, i, j, symbol);
}

lisp_object_t *make_op_pop(void) {
  return mkopcode(POP, 0);
}

lisp_object_t *make_op_prim(lisp_object_t *nargs) {
  return mkopcode(PRIM, 1, nargs);
}

lisp_object_t *make_op_return(void) {
  return mkopcode(RETURN, 0);
}

/* Type predicate */
int is_pointer(lt *object) {
  return ((int)object & POINTER_MASK) == POINTER_TAG;
}

int is_of_type(lisp_object_t *object, enum TYPE type) {
  return is_pointer(object) && (object->type == type? TRUE: FALSE);
}

int isboolean(lisp_object_t *object) {
  return is_of_type(object, BOOL);
}

#define mktype_pred(func_name, type)            \
  int func_name(lisp_object_t *object) {        \
    return is_of_type(object, type);            \
  }

mktype_pred(ischar, CHARACTER)
mktype_pred(isclose, TCLOSE)
mktype_pred(isexception, EXCEPTION)
/* mktype_pred(isfixnum, FIXNUM) */
mktype_pred(isfloat, FLOAT)
mktype_pred(isfunction, COMPILED_FUNCTION)
mktype_pred(isinput_file, INPUT_FILE)
mktype_pred(isnull, EMPTY_LIST)
mktype_pred(ispair, PAIR)
mktype_pred(isprimitive, PRIMITIVE_FUNCTION)
mktype_pred(isstring, STRING)
mktype_pred(issymbol, SYMBOL)
mktype_pred(isvector, VECTOR)
mktype_pred(isundef, UNDEF)

int isfixnum(lt *object) {
  return ((int)object & FIXNUM_MASK) == FIXNUM_TAG;
}

int isdot(lisp_object_t *object) {
  return object == dot_symbol;
}

int is_signaled(lisp_object_t *object) {
  return isexception(object) && exception_flag(object) == TRUE;
}

int isfalse(lisp_object_t *object) {
  return isboolean(object) && boolean_value(object) == FALSE? TRUE: FALSE;
}

int isnumber(lisp_object_t *object) {
  return isfixnum(object) || isfloat(object);
}

/* TODO: Moves the following section of code to above. */
/* Utilities */
lisp_object_t *booleanize(int value) {
  if (value == 0)
    return false;
  else
    return true;
}

/* TODO: Use a hash table for storing symbols. */
lisp_object_t *find_or_create_symbol(char *name) {
  lisp_object_t *tmp_sym_list = symbol_list;
  while (ispair(tmp_sym_list)) {
    lisp_object_t *sym = pair_head(tmp_sym_list);
    char *key = symbol_name(sym);
    if (strcmp(key, name) == 0)
      return sym;
    tmp_sym_list = pair_tail(tmp_sym_list);
  }
  lisp_object_t *sym = make_symbol(name);
  symbol_list = make_pair(sym, symbol_list);
  return sym;
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

int typeof(lisp_object_t *x) {
  if (isfixnum(x))
    return FIXNUM;
  assert(is_pointer(x));
  return x->type;
}

/* StringBuilder */
typedef struct string_builder_t {
  char *string;
  int length;
  int index;
} string_builder_t;

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
  /* switch (x->type) { */
  switch(typeof(x)) {
    case BOOL:
      if (boolean_value(x) == TRUE)
        write_raw_string("#t", output_file);
      if (boolean_value(x) == FALSE)
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
    case COMPILED_FUNCTION: {
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
      writef(output_file, "ERROR: %s", make_string(exception_msg(x)));
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
      writef(output_file, "#<PRIMITIVE-FUNCTION %s %p>", 
             make_string(primitive_Lisp_name(x)), x);
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
      /* Opcodes */
    case OPCODE: write_opcode(x, output_file); break;
    default :
      fprintf(stdout, "invalid object with type %d", typeof(x));
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
  switch (typeof(n)) {
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

  switch (typeof(object)) {
    mktype(BOOL);
    mktype(CHARACTER);
    mktype(FIXNUM);
    mktype(FLOAT);
    mktype(COMPILED_FUNCTION);
    mktype(PRIMITIVE_FUNCTION);
    default :
      fprintf(stdout, "Unknown type %d of object\n", typeof(object));
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
    case EOF: return make_eof();
    case '\n': input_file_linum(input_file)++;
    case ' ': return read_object(input_file);
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
        case '\\': return read_character(input_file);
        case 't': 
          if (isdelimiter(peek_char(input_file)))
            return true;
        case 'f': 
          if (isdelimiter(peek_char(input_file)))
            return false;
        default : {
          return reader_error("Unexpected character '%c' after '#', at line %d, column %d", c, input_file_linum(input_file), input_file_colnum(input_file));
        }
      }
    case '"': return read_string(input_file);
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
    case ']': case ')': return make_close();
    case '.': return dot_symbol;
    case '[': return read_vector(input_file);
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

/* Compiler */
int islength1(lisp_object_t *list) {
  return isnull(pair_tail(list));
}

int is_tag_list(lisp_object_t *object, lisp_object_t *tag) {
  return ispair(object) && (pair_head(object) == tag);
}

int is_label(lisp_object_t *object) {
  return issymbol(object);
}

lisp_object_t *asm_first_pass(lisp_object_t *code) {
  int length = 0;
  lisp_object_t *labels = null_list;
  while (!isnull(code)) {
    lisp_object_t *ins = pair_head(code);
    if (is_label(ins))
      labels = make_pair(make_pair(ins, make_fixnum(length)), labels);
    else
      length++;
    code = pair_tail(code);
  }
  return make_pair(make_fixnum(length), labels);
}

int is_addr_op(lisp_object_t *op) {
  switch (opcode_type(op)) {
    case JUMP: case FJUMP: return TRUE;
    default : return FALSE;
  }
}

lisp_object_t *get_offset(lisp_object_t *label, lisp_object_t *labels) {
  lisp_object_t *last_labels = labels;
  while (!isnull(labels)) {
    lisp_object_t *lo = pair_head(labels);
    if (!isfalse(lt_eq(label, pair_head(lo))))
      return pair_tail(lo);
    labels = pair_tail(labels);
  }
  fprintf(stdout, "Impossible!!! %s not found!\n", symbol_name(label));
  write_expr("labels", last_labels);
  exit(1);
}

lisp_object_t *change_addr(lisp_object_t *ins, lisp_object_t *table) {
  switch (opcode_type(ins)) {
    case FJUMP: {
      lisp_object_t *label = op_fjump_label(ins);
      return make_op_fjump(get_offset(label, table));
    }
    case JUMP: {
      lisp_object_t *label = op_jump_label(ins);
      return make_op_jump(get_offset(label, table));
    }
    default :
      fprintf(stdout, "Invalid instruction to change address\n");
      exit(1);
  }
}

lisp_object_t *asm_second_pass(lisp_object_t *code, lisp_object_t *length, lisp_object_t *labels) {
  int index = 0;
  lisp_object_t *code_vector = make_vector(fixnum_value(length));
  while (!isnull(code)) {
    lisp_object_t *ins = pair_head(code);
    if (!is_label(ins)) {
      if (is_addr_op(ins))
        ins = change_addr(ins, labels);
      vector_value(code_vector)[index] = ins;
      index++;
    }
    code = pair_tail(code);
  }
  return code_vector;
}

lisp_object_t *assemble(lisp_object_t *func) {
  lisp_object_t *ll = asm_first_pass(function_code(func));
  lisp_object_t *length = pair_head(ll);
  lisp_object_t *labels = pair_tail(ll);
  function_code(func) = asm_second_pass(function_code(func), length, labels);
  return func;
}

lisp_object_t *gen(enum TYPE opcode, ...) {
  va_list ap;
  va_start(ap, opcode);
  lisp_object_t *ins;
  switch (opcode) {
    case ARGS: {
      lisp_object_t *length = va_arg(ap, lisp_object_t *);
      ins = make_op_args(length);
    }
      break;
    case CALL: ins = make_op_call(va_arg(ap, lisp_object_t *)); break;
    case FJUMP: {
      lisp_object_t *label = va_arg(ap, lisp_object_t *);
      ins = make_op_fjump(label);
    }
      break;
    case FN: ins = make_op_fn(va_arg(ap, lisp_object_t *)); break;
    case GSET: ins = make_op_gset(va_arg(ap, lisp_object_t *)); break;
    case GVAR: ins = make_op_gvar(va_arg(ap, lisp_object_t *)); break;
    case JUMP: {
      lisp_object_t *label = va_arg(ap, lisp_object_t *);
      ins = make_op_jump(label);
    }
      break;
    case LSET: {
      lisp_object_t *i = va_arg(ap, lisp_object_t *);
      lisp_object_t *j = va_arg(ap, lisp_object_t *);
      lisp_object_t *symbol = va_arg(ap, lisp_object_t *);
      ins = make_op_lset(i, j, symbol);
    }
      break;
    case LVAR: {
      lisp_object_t *i = va_arg(ap, lisp_object_t *);
      lisp_object_t *j = va_arg(ap, lisp_object_t *);
      lisp_object_t *symbol = va_arg(ap, lisp_object_t *);
      ins = make_op_lvar(i, j, symbol);
    }
      break;
    case CONST: {
      lisp_object_t *value = va_arg(ap, lisp_object_t *);
      ins = make_op_const(value);
    }
      break;
    case POP: ins = make_op_pop(); break;
    case PRIM: ins = make_op_prim(va_arg(ap, lisp_object_t *)); break;
    case RETURN: ins = make_op_return(); break;
    default :
      fprintf(stdout, "Invalid opcode %d\n", opcode);
      exit(1);
  }
  return make_pair(ins, make_empty_list());
}

lisp_object_t *compile_begin(lisp_object_t *exps, lisp_object_t *env) {
  if (isnull(exps))
    return gen(CONST, null_list);
  else if (islength1(exps))
    return compile_object(first(exps), env);
  else {
    lisp_object_t *st = compile_object(first(exps), env);
    lisp_object_t *nd = gen(POP);
    lisp_object_t *rd = compile_begin(pair_tail(exps), env);
    return seq(st, nd, rd);
  }
}

lisp_object_t *make_label(void) {
  static int label_count = 1;
  static char buffer[256];
  int i = sprintf(buffer, "L%d", label_count);
  label_count++;
  return find_or_create_symbol(strndup(buffer, i));
}

lisp_object_t *compile_if(lisp_object_t *pred, lisp_object_t *then, lisp_object_t *else_part, lisp_object_t *env) {
  lisp_object_t *l1 = make_label();
  lisp_object_t *l2 = make_label();
  pred = compile_object(pred, env);
  then = compile_object(then, env);
  else_part = compile_object(else_part, env);
  lisp_object_t *fj = gen(FJUMP, l1);
  lisp_object_t *j = gen(JUMP, l2);
  return seq(pred, fj, then, j, list1(l1), else_part, list1(l2));
}

int is_all_symbol(lisp_object_t *list) {
  while (!isnull(list)) {
    if (!issymbol(pair_head(list)))
      return FALSE;
    list = pair_tail(list);
  }
  return TRUE;
}

lisp_object_t *compile_lambda(lisp_object_t *args, lisp_object_t *body, lisp_object_t *env) {
  assert(is_all_symbol(args));
  lisp_object_t *len = lt_list_length(args);
  lisp_object_t *code = seq(gen(ARGS, len),
                            compile_begin(body, make_pair(args, env)),
                            gen(RETURN));
  lisp_object_t *func = make_function(env, args, code);
  return assemble(func);
}

lisp_object_t *compile_args(lisp_object_t *args, lisp_object_t *env) {
  if (isnull(args))
    return null_list;
  else
    return lt_append2(compile_object(pair_head(args), env),
                      compile_args(pair_tail(args), env));
}

lisp_object_t *is_var_in_frame(lisp_object_t *var, lisp_object_t *bindings) {
  assert(isnull(bindings) || ispair(bindings) || isvector(bindings));
  if (ispair(bindings)) {
    int j = 0;
    while (!isnull(bindings)) {
      if (!isfalse(lt_eq(var, pair_head(bindings))))
        return make_fixnum(j);
      bindings = pair_tail(bindings);
      j++;
    }
  }
  if (isvector(bindings)) {
    printf("Searching %s in vector...\n", symbol_name(var));
    for (int j = 0; j < vector_length(bindings); j++) {
      if (!isfalse(lt_eq(var, vector_value(bindings)[j])))
        return make_fixnum(j);
    }
  }
  return NULL;
}

lisp_object_t *find_in_frame(lisp_object_t *bindings, int j) {
  assert(ispair(bindings) || isvector(bindings));
  if (ispair(bindings)) {
    bindings = lt_raw_nthtail(bindings, j);
    return pair_head(bindings);
  } else if (isvector(bindings)) {
    return vector_value(bindings)[j];
  }
  fprintf(stdout, "Impossible!!! variable not found!\n");
  exit(1);
}

void set_in_frame(lisp_object_t *bindings, int j, lisp_object_t *value) {
  assert(ispair(bindings) || isvector(bindings));
  if (ispair(bindings)) {
    bindings = lt_raw_nthtail(bindings, j);
    pair_head(bindings) = value;
    return;
  } else if (isvector(bindings)) {
    vector_value(bindings)[j] = value;
    return;
  }
  fprintf(stdout, "Impossible!!! variable not found!\n");
  exit(1);
}

lisp_object_t *is_var_in_env(lisp_object_t *symbol, lisp_object_t *env) {
  assert(issymbol(symbol));
  int i = 0;
  while (env != null_env) {
    lisp_object_t *bindings = pair_head(env);
    assert(isnull(bindings) ||ispair(bindings) || isvector(bindings));
    lisp_object_t *j = is_var_in_frame(symbol, bindings);
    if (j != NULL)
      return make_pair(make_fixnum(i), j);
    env = pair_tail(env);
    i++;
  }
  return NULL;
}

lt *walk_in_env(lt *env, int n) {
  while (n-- > 0)
    env = pair_tail(env);
  return env;
}

lisp_object_t *locate_var(lisp_object_t *env, int i, int j) {
  env = walk_in_env(env, i);
  return find_in_frame(pair_head(env), j);
}

void set_local_var(lisp_object_t *env, int i, int j, lisp_object_t *value) {
  env = walk_in_env(env, i);
  set_in_frame(pair_head(env), j, value);
}

lisp_object_t *gen_var(lisp_object_t *symbol, lisp_object_t *env) {
  lisp_object_t *co = is_var_in_env(symbol, env);
  if (co == NULL)
    return gen(GVAR, symbol);
  else {
    lisp_object_t *i = pair_head(co);
    lisp_object_t *j = pair_tail(co);
    return gen(LVAR, i, j, symbol);
  }
}

lisp_object_t *gen_set(lisp_object_t *symbol, lisp_object_t *env) {
  lisp_object_t *co = is_var_in_env(symbol, env);
  if (co == NULL)
    return gen(GSET, symbol);
  else {
    lisp_object_t *i = pair_head(co);
    lisp_object_t *j = pair_tail(co);
    return gen(LSET, i, j, symbol);
  }
}

/* TODO: The support for built-in macros. */
/* TODO: The support for tail call optimization. */
pub lisp_object_t *compile_object(lisp_object_t *object, lisp_object_t *env) {
  if (issymbol(object))
    return gen_var(object, env);
  if (!ispair(object))
    return gen(CONST, object);
  if (is_tag_list(object, S("quote")))
    return gen(CONST, second(object));
  if (is_tag_list(object, S("begin")))
    return compile_begin(pair_tail(object), env);
  if (is_tag_list(object, S("set!"))) {
    lisp_object_t *value = compile_object(third(object), env);
    lisp_object_t *set = gen_set(second(object), env);
    return seq(value, set);
  }
  if (is_tag_list(object, S("if"))) {
    lisp_object_t *pred = second(object);
    lisp_object_t *then = third(object);
    lisp_object_t *else_part = fourth(object);
    return compile_if(pred, then, else_part, env);
  }
  if (is_tag_list(object, S("fn")))
    return gen(FN, compile_lambda(second(object), pair_tail(pair_tail(object)), env));
  if (ispair(object)) {
    lisp_object_t *args = pair_tail(object);
    lisp_object_t *fn = pair_head(object);
    assert(issymbol(fn));
    /* Generating different instruction when calling primitive and compiled function */
    if (is_symbol_bound(fn) && isprimitive(symbol_value(fn)))
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(PRIM, lt_list_length(args)));
    else
      return seq(compile_args(args, env),
                 compile_object(fn, env),
                 gen(CALL, lt_list_length(args)));
  } else {
    fprintf(stdout, "Invalid expression to be compiled\n");
    exit(1);
  }
}

lisp_object_t *compile_as_lambda(lisp_object_t *form) {
  lisp_object_t *result = compile_lambda(make_empty_list(), list1(form), null_env);
  return result;
}

/* Virtual Machine */
lisp_object_t *raw_vector_ref(lisp_object_t *vector, int index) {
  assert(isvector(vector));
  return vector_value(vector)[index];
}

/* TODO: Exception signaling and handling. */
pub lisp_object_t *run_by_llam(lisp_object_t *func) {

#define vlast(v, n) lt_vector_last_nth(v, make_fixnum(n))

  assert(isfunction(func));
  int pc = 0;
  lisp_object_t *stack = make_vector(10);
  lisp_object_t *code = function_code(func);
  lisp_object_t *env = null_env;
  lisp_object_t *return_stack = null_list;
  while (pc < vector_length(code)) {
    lisp_object_t *ins = raw_vector_ref(code, pc);
    if (debug_flag)
      writef(standard_out, "In #run_by_llam --- executing instruction %?\n", ins);
    switch (opcode_type(ins)) {
      case ARGS: {
        lisp_object_t *args = make_vector(fixnum_value(op_args_arity(ins)));
        for (int i = fixnum_value(op_args_arity(ins)) - 1; i >= 0; i--) {
          lisp_object_t *arg = lt_vector_pop(stack);
          vector_value(args)[i] = arg;
        }
        env = make_pair(args, env);
      }
        break;
      case CALL: {
        lisp_object_t *func = lt_vector_pop(stack);
        assert(isfunction(func));
        lisp_object_t *retaddr = make_retaddr(code, env, pc);
        return_stack = make_pair(retaddr, return_stack);
        code = function_code(func);
        env = function_env(func);
        pc = -1;
      }
        break;
      case CONST:
        if (debug_flag)
          writef(standard_out, "In #run_by_llam --- pushing %? to stack\n", op_const_value(ins));
        lt_vector_push(stack, op_const_value(ins));
        break;
      case FJUMP:
        if (isfalse(lt_vector_pop(stack)))
          pc = fixnum_value(op_fjump_label(ins)) - 1;
        break;
      case FN: {
        if (debug_flag)
          writef(standard_out, "In #run_by_llam --- making function with environment %p\n%?\n", env, env);
        lisp_object_t *func = op_fn_func(ins);
        func = make_function(env, null_list, function_code(func));
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
        if (symbol_value(sym) == undef_object) {
          fprintf(stdout, "Undefined global variable: %s\n", symbol_name(sym));
          exit(1);
        }
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
      case POP: lt_vector_pop(stack); break;
      case PRIM: {
        lisp_object_t *func = lt_vector_pop(stack);
        lisp_object_t *val = NULL;
        assert(isprimitive(func));

#define _arg(N) vlast(stack, primitive_arity(func) - N)
#define _arg1 _arg(1)
#define _arg2 _arg(2)
#define _arg3 _arg(3)
#define move_stack() vector_last(stack) -= primitive_arity(func)

#define check_exception()                       \
        do {                                    \
          if (is_signaled(val)) {               \
            write_object(val, standard_out);    \
            lt_vector_push(stack, null_list);   \
            goto halt;                          \
          }                                     \
        } while (0)

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
        if (is_signaled(vlast(stack, 0)))
          goto halt;
      }
        break;
      case RETURN: {
        if (isnull(return_stack))
          break;
        lisp_object_t *retaddr = pair_head(return_stack);
        return_stack = pair_tail(return_stack);
        code = retaddr_code(retaddr);
        env = retaddr_env(retaddr);
        pc = retaddr_pc(retaddr);
      }
        break;
      default :
        fprintf(stdout, "In run_by_llam --- Invalid opcode %d\n", /* ins->type */typeof(ins));
        exit(1);
    }
    pc++;
    if (debug_flag)
      writef(standard_out, "In #run_by_llam --- after executing, the stack is\n%?\n", stack);
  }
halt:
  assert(isfalse(lt_is_vector_empty(stack)));
  return vlast(stack, 0);
}

/* Initialization */
void init_object_pool(void) {
  static int flag = 0;
  if (flag == 1) return;
  flag = 1;
  object_pool = calloc(OBJECT_INIT_COUNT, sizeof(lt));
  for (int i = 0; i < OBJECT_INIT_COUNT - 1; i++)
    object_pool[i].next = &object_pool[i + 1];
  object_pool[OBJECT_INIT_COUNT - 1].next = NULL;
  free_objects = &object_pool[0];
  used_objects = NULL;
}

void init_global_variable(void) {
  init_object_pool();
  /* Initialize global variables */
  debug_flag = FALSE;
  false = make_boolean(FALSE);
  true = make_boolean(TRUE);
  null_list = make_empty_list();
  null_env = null_list;
  standard_in = make_input_file(stdin);
  standard_out = make_output_file(stdout);
  symbol_list = null_list;
  undef_object = make_undef();
  /* Symbol initialization */
  dot_symbol = S(".");

  symbol_value(S("*standard-output*")) = standard_out;
  symbol_value(S("*standard-input*")) = standard_in;

  lisp_object_t *func;

#define ADD(arity, function_name, Lisp_name)                            \
  do {                                                                  \
    func =                                                              \
        make_primitive(arity, (void *)function_name, Lisp_name);        \
    symbol_value(S(Lisp_name)) = func;                                  \
  } while (0)

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
  if (debug_flag)
    writef(standard_out, "Initialization finished.\n");
}

/* Driver */
int main(int argc, char *argv[])
{
  char *inputs[] = {
    "(set! abs (fn (x) (if (> 0 x) (- 0 x) x)))",
    "(abs 1)",
    "(abs -1)",
  };
  init_global_variable();
  for (int i = 0; i < sizeof(inputs) / sizeof(char *); i++) {
    lisp_object_t *expr = read_object_from_string(inputs[i]);
    if (debug_flag)
      writef(standard_out, "In #main --- expr is %?\n", expr);
    expr = compile_as_lambda(expr);
    if (debug_flag)
      writef(standard_out, "In #main --- compile expr is\n%?\n", expr);
    writef(standard_out, ">> %s\n", make_string(inputs[i]));
    expr = run_by_llam(expr);
    if (is_signaled(expr))
      writef(standard_out, "%?\n", expr);
    else
      writef(standard_out, "=> %?\n", expr);
  }
  return 0;
}
