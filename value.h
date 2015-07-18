#ifndef VALUE_H_
#define VALUE_H_

#include <stdio.h>
#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __value_t value_t;

typedef enum {
    VALUE_INVALID,
    VALUE_INT,
} value_kind_t;

struct __value_t {
    value_kind_t kind;
    union {
        int int_val;
        string_t *invalid_msg;
    } u;
};

extern value_t *value_invalid_new(const char *msg);
extern value_t *value_int_new(int);
extern void value_free(value_t *);
extern void value_print(value_t *, FILE *);

#ifdef __cplusplus
}
#endif

#endif /* VALUE_H_ */
