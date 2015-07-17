/*
 * string.h
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#ifndef STRING_H_
#define STRING_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __string_t string_t;

struct __string_t {
    char *text;
    size_t capacity;
    size_t length;
};

extern string_t *string_new(void);
extern void string_free(string_t *);
extern void string_addc(string_t *, char);
extern void string_assign(string_t *, const char *);
extern void string_clear(string_t *);

#ifdef __cplusplus
}
#endif

#endif /* STRING_H_ */
