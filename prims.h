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

extern void write_object(lt *, lt *);
extern void write_raw_char(char, lt *);
extern void write_raw_string(char *, lt *);
extern void writef(lt *, const char *, ...);

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
extern F2(lt_vector_push_extend);
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
extern F1(lt_load);

extern lt *read_object_from_string(char *);
extern lt *read_object(lt *);

extern void init_prims(void);
extern void load_init_file(void);

#define seq(...) append_n(__VA_ARGS__, NULL)

#define first(x) lt_raw_nth((x), 0)
#define second(x) lt_raw_nth((x), 1)
#define third(x) lt_raw_nth((x), 2)
#define fourth(x) lt_raw_nth((x), 3)

#endif /* PRIMS_H_ */
