/*
 * utilities.h
 *
 *  Created on: 2013年7月20日
 *      Author: liutos
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stdint.h>

#include "type.h"

extern lt *booleanize(int);
extern int is_label(lt *);
extern int is_macro_form(lt *);
extern int is_symbol_bound(lt *);

extern lt *list1(lt *);
extern lt *list2(lt *, lt *);
extern lt *list3(lt *, lt *, lt *);
extern lt *list4(lt *, lt *, lt *, lt *e);
extern lt *append2(lt *, lt *);
extern lt *append_n(lt *, ...);
extern lt *lt_raw_nth(lt *, int);
extern lt *lt_raw_nthtail(lt *, int);
extern int pair_length(lt *);
extern lt *raw_list(lt *, ...);
extern lt *reader_error(char *, ...);
extern void sb_add_char(string_builder_t *, char);
extern char *sb2string(string_builder_t *);
extern lt *signal_exception(char *);
extern lt *signal_typerr(char *);

/* UTF-8 */
extern int count1(char);
extern char *code_point_to_utf8(uint32_t);
extern lt *make_unicode_char(char);
extern uint32_t get_code_point(char *);

/* Opcode */
extern void set_op4prim(lt *, enum OPCODE_TYPE);
extern hash_table_t *make_prim2op_map(void);
extern lt *make_op_call(lt *);
extern lt *make_op_checkex(void);
extern lt *make_op_chkarity(lt *);
extern lt *make_op_chktype(lt *, lt *, lt *);
extern lt *make_op_const(lt *);
extern lt *make_op_extenv(lt *);
extern lt *make_op_fjump(lt *);
extern lt *make_op_fn(lt *);
extern lt *make_op_gset(lt *);
extern lt *make_op_gvar(lt *);
extern lt *make_op_jump(lt *);
extern lt *make_op_lset(lt *i, lt *j, lt *symbol);
extern lt *make_op_lvar(lt *i, lt *j, lt *symbol);
extern lt *make_op_moveargs(lt *);
extern lt *make_op_pop(void);
extern lt *make_op_popenv(void);
extern lt *make_op_prim(lt *);
extern lt *make_op_restargs(lt *);
extern lt *make_op_return(void);
extern lt *make_op_catch(void);
extern lt *make_fn_inst(lt *);
extern lt *search_op4prim(lt *);

/* Package */
extern void use_package_in(lt *, lt *);
extern lt *ensure_package(char *);
extern lt *search_package(char *, lt *);

/* String */
/** Export **/
extern char *export_C_string(lt *);
/** Import **/
extern lt *import_C_string(char *);

/* String Builder */
extern string_builder_t *make_str_builder(void);

/* Structure */
extern int compute_field_offset(char *, char *);
extern void set_structure(char *, lt *);
extern hash_table_t *make_structures_table(void);
extern lt *search_structure(char *);

/* Symbol */
extern hash_table_t *make_symbol_table(void);
extern lt *find_or_create_symbol(char *, lt *);
#define LISP(name) find_or_create_symbol(name, pkg_lisp)
#define S(name) (find_or_create_symbol(name, package))

/* Special Forms */
extern int is_tag_list(lt *list, lt *tag);
extern int is_begin_form(lt *);
extern int is_catch_form(lt *);
extern int is_goto_form(lt *);
extern int is_if_form(lt *);
extern int is_lambda_form(lt *);
extern int is_let_form(lt *);
extern int is_quote_form(lt *);
extern int is_return_form(lt *);
extern int is_set_form(lt *);
extern int is_tagbody_form(lt *);
/** LET **/
extern lt *let_bindings(lt *);
extern lt *let_body(lt *);
extern lt *let_vals(lt *);
extern lt *let_vars(lt *);

#endif /* UTILITIES_H_ */
