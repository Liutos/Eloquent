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
  if (n == 0)
    return *str;
  else {
    uint32_t cp = get_low_bits(str[0], 8 - (n + 1));
    for (int i = 1; i < n; i++)
      cp = (cp << 6) + get_low_bits(str[i], 6);
    return cp;
  }
}

int bytes_need(uint32_t cp) {
  if (cp < 0x80)
    return 1;
  if (cp < 0x800)
    return 2;
  if (cp < 0x10000)
    return 3;
  if (cp < 0x200000)
    return 4;
  else {
    printf("Error happens\n");
    exit(1);
  }
}

char *code_point_to_utf8(uint32_t cp) {
  static const int MS1[] = {0x00, 0xC0, 0xE0, 0xF0};
  static const int MS2[] = {0xFF, 0x1F, 0x0F, 0x07};
  int n = bytes_need(cp);
  char *str = calloc(n, sizeof(char));
  for (int i = n - 1; i > 0; i--) {
    str[i] = 0x80 | (cp & 0x3F);
    cp = cp >> 6;
  }
  str[0] = MS1[n - 1] | (cp & MS2[n - 1]);
  return str;
}

/* Boolean */
lt *booleanize(int value) {
  if (value == 0)
    return the_false;
  else
    return the_true;
}

/* Exception */
lisp_object_t *reader_error(char *format, ...) {
  static char msg[1000];
  va_list ap;
  va_start(ap, format);
  vsprintf(msg, format, ap);
  return make_exception(strdup(msg), TRUE, the_reader_error_symbol, the_empty_list);
}

lt *signal_exception(char *message) {
  return make_exception(message, TRUE, the_error_symbol, the_empty_list);
}

lt *signal_typerr(char *type_name) {
  char msg[256];
  sprintf(msg, "Argument is not of type %s", type_name);
  return signal_exception(strdup(msg));
}

/* List */
int is_tag_list(lisp_object_t *object, lisp_object_t *tag) {
  return is_lt_pair(object) && (pair_head(object) == tag);
}

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

/* Symbol */
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

/* String Builder */
string_builder_t *make_str_builder(void) {
  string_builder_t *sb = GC_MALLOC(sizeof(*sb));
  sb->length = 20;
  sb->string = GC_MALLOC(sb->length * sizeof(char));
  sb->index = 0;
  return sb;
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

/* Opcode */
/* Opcode constructor functions */
lt *mkopcode(enum OPCODE_TYPE name, int arity, ...) {
  lt **oprands = GC_MALLOC(arity * sizeof(lt *));
  va_list ap;
  va_start(ap, arity);
  for (int i = 0; i < arity; i++)
    oprands[i] = va_arg(ap, lt *);
  return make_opcode(name, arity, opcode_op(opcode_ref(name)), oprands);
}

lisp_object_t *make_op_call(lisp_object_t *arity) {
  assert(isfixnum(arity));
  return mkopcode(CALL, 1, arity);
}

lt *make_op_checkex(void) {
  return mkopcode(CHECKEX, 0);
}

lt *make_op_chkarity(lt *arity) {
  assert(isfixnum(arity));
  return mkopcode(CHKARITY, 1, arity);
}

lt *make_op_chktype(lt *position, lt *target_type, lt *nargs) {
  assert(isfixnum(position));
  assert(isfixnum(nargs));
  return mkopcode(CHKTYPE, 3, position, target_type, nargs);
}

lisp_object_t *make_op_const(lisp_object_t *value) {
  return mkopcode(CONST, 1, value);
}

lt *make_op_extenv(lt *count) {
  assert(isfixnum(count));
  return mkopcode(EXTENV, 1, count);
}

lisp_object_t *make_op_fjump(lisp_object_t *label) {
  assert(is_lt_symbol(label) || isfixnum(label));
  return mkopcode(FJUMP, 1, label);
}

lisp_object_t *make_op_fn(lisp_object_t *func) {
  assert(is_lt_function(func));
  return mkopcode(FN, 1, func);
}

lisp_object_t *make_op_gset(lisp_object_t *symbol) {
  assert(is_lt_symbol(symbol));
  return mkopcode(GSET, 1, symbol);
}

lisp_object_t *make_op_gvar(lisp_object_t *symbol) {
  assert(is_lt_symbol(symbol));
  return mkopcode(GVAR, 1, symbol);
}

lisp_object_t *make_op_jump(lisp_object_t *label) {
  assert(is_lt_symbol(label) || isfixnum(label));
  return mkopcode(JUMP, 1, label);
}

lt *make_op_lset(lt *i, lt *j, lt *symbol) {
  assert(isfixnum(i));
  assert(isfixnum(j));
  assert(is_lt_symbol(symbol));
  return mkopcode(LSET, 3, i, j, symbol);
}

lt *make_op_lvar(lt *i, lt *j, lt *symbol) {
  assert(isfixnum(i));
  assert(isfixnum(j));
  assert(is_lt_symbol(symbol));
  return mkopcode(LVAR, 3, i, j, symbol);
}

lt *make_op_moveargs(lt *count) {
  assert(isfixnum(count));
  return mkopcode(MOVEARGS, 1, count);
}

lisp_object_t *make_op_pop(void) {
  return mkopcode(POP, 0);
}

lt *make_op_popenv(void) {
  return mkopcode(POPENV, 0);
}

lisp_object_t *make_op_prim(lisp_object_t *nargs) {
  assert(isfixnum(nargs));
  return mkopcode(PRIM, 1, nargs);
}

lisp_object_t *make_op_return(void) {
  return mkopcode(RETURN, 0);
}

lt *make_op_restargs(lt *count) {
  assert(isfixnum(count));
  return mkopcode(RESTARGS, 1, count);
}

lt *make_op_catch(void) {
  return mkopcode(CATCH, 0);
}

int prim_comp_fn(void *p1, void *p2) {
  return p1 - p2;
}

unsigned int prim_hash_fn(void *prim) {
  return (unsigned int)prim;
}

hash_table_t *make_prim2op_map(void) {
  return make_hash_table(31, prim_hash_fn, prim_comp_fn);
}

lt *search_op4prim(lt *prim) {
  assert(is_lt_primitive(prim));
  return search_ht(prim, prim2op_map);
}

void set_op4prim(lt *prim, enum OPCODE_TYPE opcode) {
  set_ht(prim, opcode_ref(opcode), prim2op_map);
}

int isopcode_fn(lt *prim) {
  assert(is_lt_primitive(prim));
  return search_op4prim(prim) != NULL;
}

lt *make_fn_inst(lt *prim) {
  assert(is_lt_primitive(prim));
  lt *opcode = search_op4prim(prim);
  assert(opcode != NULL);
  return make_pair(mkopcode(opcode_name(opcode), 0), the_empty_list);
}

/* Package */
lt *search_package(char *name, lt *packages) {
  while (is_lt_pair(packages)) {
    lt *pkg = pair_head(packages);
    if (strcmp(export_C_string(package_name(pkg)), name) == 0)
      return pkg;
    packages = pair_tail(packages);
  }
  return NULL;
}

lt *ensure_package(char *name) {
  lt *result = search_package(name, pkgs);
  if (result)
    return result;
  lt *pkg = make_package(import_C_string(name), make_symbol_table());
  pkgs = make_pair(pkg, pkgs);
  symbol_value(find_or_create_symbol("*package*", pkg)) = pkg;
  return pkg;
}

void use_package_in(lt *used, lt *pkg) {
  package_used_packages(pkg) =
      make_pair(used, package_used_packages(pkg));
}

/* String */
/** Export: Code Point -> UTF-8 **/
int compute_nbytes(uint32_t *value, int length) {
  int n = 0;
  for (int i = 0; i < length; i++)
    n += bytes_need(value[i]);
  return n;
}

// Get the C string from a Lisp string
char *code_point_to_C_string(uint32_t *value, int length) {
  length = compute_nbytes(value, length);
  char *str = GC_MALLOC((length + 1) * sizeof(char));
  str[length] = '\0';
  int offset = 0;
  for (int i = 0; value[i] != 0; i++) {
    char *tmp = code_point_to_utf8(value[i]);
    memcpy(str + offset, tmp, count1(*tmp));
    offset += bytes_need(value[i]);
  }
  return str;
}

char *export_C_string(lt *string) {
  return code_point_to_C_string(string_value(string), string_length(string));
}

/** Import: UTF-8 -> Code Point **/
int C_string_count(char *str) {
  int count = 0;
  int i = 0;
  while (str[i] != '\0') {
    i += count1(str[i]);
    count++;
  }
  return count;
}

// Convert each UTF-8 character in `str' to a number of type `uint32_t'
uint32_t *C_string_to_code_point(char *str) {
  int len = C_string_count(str);
  uint32_t *value = GC_MALLOC((len + 1) * sizeof(uint32_t));
  value[len] = 0;
  int k = 0;
  for (int i = 0; i < len; i++) {
    value[i] = get_code_point(&str[k]);
    k += count1(str[k]);
  }
  return value;
}

int code_point_count(uint32_t *value) {
  int len = 0;
  while (*value != 0) {
    len++;
    value++;
  }
  return len;
}

lt *import_C_string(char *C_str) {
  uint32_t *cps = C_string_to_code_point(C_str);
  int length = code_point_count(cps);
  return make_string(length, cps);
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

unsigned int struct_hash_fn(void *struct_name) {
  return string_hash_fn(struct_name);
}

int struct_comp_fn(void *s1, void *s2) {
  return string_comp_fn(s1, s2);
}

lt *search_structure(char *struct_name) {
  return search_ht(struct_name, st_tbl);
}

void set_structure(char *name, lt *fields) {
  set_ht(name, fields, st_tbl);
}

hash_table_t *make_structures_table(void) {
  return make_hash_table(31, struct_hash_fn, struct_comp_fn);
}

/* Symbol */
// The following algorithm comes from http://bbs.csdn.net/topics/350030230
unsigned int symbol_hash_fn(void *symbol) {
  return string_hash_fn(symbol);
}

int symbol_comp_fn(void *s1, void *s2) {
  return string_comp_fn(s1, s2);
}

hash_table_t *make_symbol_table(void) {
  return make_hash_table(31, symbol_hash_fn, symbol_comp_fn);
}

// Search the symbol with `name' in `symbol_table'
lt *search_symbol_table(char *name, hash_table_t *symbol_table) {
  return search_ht((void *)name, symbol_table);
}

lt *find_symbol(char *name, lt *package) {
  lt *sym = search_symbol_table(name, package_symbol_table(package));
  if (sym)
    return sym;
  lt *useds = package_used_packages(package);
  while (is_lt_pair(useds)) {
    lt *pkg = pair_head(useds);
    sym = search_symbol_table(name, package_symbol_table(pkg));
    if (sym)
      return sym;
    useds = pair_tail(useds);
  }
  return NULL;
}

lt *find_or_create_symbol(char *name, lt *package) {
  lt *result = find_symbol(name, package);
  if (result)
    return result;
  lt *sym = make_symbol(name, package);
  set_ht((void *)name, (void *)sym, package_symbol_table(package));
  return sym;
}

/* Special Forms */
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
deform_pred(is_return_form, "return");
deform_pred(is_set_form, "set!")
deform_pred(is_tagbody_form, "tagbody")

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
