/*
 * utilities.h
 *
 *  Created on: 2013年7月20日
 *      Author: liutos
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "type.h"

extern lt *booleanize(int);
extern int is_label(lt *);
extern int is_macro_form(lt *);
extern int is_symbol_bound(lt *);
extern int is_tag_list(lt *list, lt *tag);
extern lt *list1(lt *);
extern lt *list2(lt *, lt *);
extern lt *list3(lt *, lt *, lt *);
extern lt *append2(lt *, lt *);
extern lt *append_n(lt *, ...);
extern lt *lt_raw_nth(lt *, int);
extern lt *lt_raw_nthtail(lt *, int);
extern int pair_length(lt *);
extern lt *reader_error(char *, ...);
extern void sb_add_char(string_builder_t *, char);
extern char *sb2string(string_builder_t *);
extern lt *signal_exception(char *);
extern lt *signal_typerr(char *);

#endif /* UTILITIES_H_ */
