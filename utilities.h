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

extern int is_tag_list(lt *list, lt *tag);
extern int is_begin_form(lt *);
extern int is_catch_form(lt *);
extern int is_goto_form(lt *);
extern int is_if_form(lt *);
extern int is_lambda_form(lt *);
extern int is_let_form(lt *);
extern int is_quote_form(lt *);
extern int is_set_form(lt *);
extern int is_tagbody_form(lt *);

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

/* Structure */
extern int compute_field_offset(char *, char *);

/* Special Forms */
/** LET **/
extern lt *let_bindings(lt *);
extern lt *let_body(lt *);
extern lt *let_vals(lt *);
extern lt *let_vars(lt *);

/* UTF-8 */
extern int count1(char);
extern char *code_point_to_utf8(uint32_t);
extern lt *make_unicode_char(char);
extern uint32_t get_code_point(char *);
/* String */
extern int C_string_count(char *);
extern int get_string_length(uint32_t *);
extern char *C_string(lt *);
extern char *to_C_string(uint32_t *, int);
extern uint32_t *utf8s_to_code_point(char *);
extern lt *wrap_C_string(char *);

#endif /* UTILITIES_H_ */
