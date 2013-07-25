/*
 * prims.h
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */

#ifndef PRIMS_H_
#define PRIMS_H_

#include "type.h"

#define F0(x) lt *x(void);
#define F1(x) lt *x(lt *);
#define F2(x) lt *x(lt *, lt *);
#define F3(x) lt *x(lt *, lt *, lt *);

extern F2(lt_add);
extern F2(lt_div);
extern F2(lt_gt);
extern F2(lt_mod);
extern F2(lt_mul);
extern F2(lt_numeric_eq);
extern F2(lt_sub);
extern F1(lt_char_code);
extern F1(lt_code_char);
extern F1(lt_read_char);
extern F1(lt_read_line);
extern F1(lt_head);
extern F1(lt_list_length);
extern F1(lt_list_nreverse);
extern F1(lt_list_reverse);
extern F2(lt_nth);
extern F2(lt_nthtail);
extern F2(lt_set_head);
extern F2(lt_set_tail);
extern F1(lt_tail);
extern F2(lt_char_at);
extern F1(lt_string_length);
extern F3(lt_string_set);
extern F1(lt_intern);
extern F1(lt_symbol_name);
extern F1(lt_symbol_value);
extern F1(lt_is_vector_empty);
extern F1(lt_list_to_vector);
extern F2(lt_vector_last_nth);
extern F1(lt_vector_pop);
extern F2(lt_vector_push);
extern F2(lt_vector_reF);
extern F3(lt_vector_set);
extern F1(lt_vector_to_list);
extern F2(lt_eq);
extern F2(lt_eql);
extern F2(lt_equal);
extern F0(lt_object_size);
extern F1(lt_type_of);
extern F1(lt_is_constant);
extern F1(lt_expand_macro);
extern F1(lt_function_arity);
extern F2(lt_simple_apply);
extern void init_macros(void);

extern lt *read_object_from_string(char *);
extern void writef(lt *, const char *, ...);
extern void write_expr(char *, lt *);
extern void init_prims(void);
extern lt *lt_append2(lt *, lt *);
extern lt *lt_raw_nth(lt *, int);
extern lt *lt_append(lt *, ...);
extern lt *lt_raw_nthtail(lt *, int);
extern void write_raw_string(char *, lt *);
extern void write_raw_char(char, lt *);
extern void write_object(lt *, lt *);

#define seq(...) lt_append(__VA_ARGS__, NULL)

#endif /* PRIMS_H_ */
