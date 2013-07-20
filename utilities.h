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
extern int is_symbol_bound(lt *);
extern lt *list1(lt *);
extern lt *signal_exception(char *);
extern lt *signal_typerr(char *);
extern int pair_length(lt *);
extern lt *reader_error(char *, ...);
extern string_builder_t *make_str_builder(void);
extern void sb_add_char(string_builder_t *, char);
extern char *sb2string(string_builder_t *);

#endif /* UTILITIES_H_ */
