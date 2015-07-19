#ifndef VALUE_H_
#define VALUE_H_

#include <stdio.h>
#include "ast.h"
#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __value_t value_t;
typedef struct __value_error_t value_error_t;
typedef struct __value_function_t value_function_t;

typedef value_t *(*bif_1)(value_t *);
typedef value_t *(*bif_2)(value_t *, value_t *);

typedef enum {
    VALUE_ERROR,
    VALUE_FUNCTION,
    VALUE_INT,
} value_kind_t;

struct __value_error_t {
    string_t *msg;
};

struct __value_function_t {
    int is_bif;
    unsigned int arity;
    union {
        void *bif_ptr;
        struct {
            ast_t *pars;
            ast_t *body;
        } udf;
    } u;
};

struct __value_t {
    value_kind_t kind;
    union {
        int int_val;
        value_error_t err_val;
        value_function_t func_val;
    } u;
};

extern value_t *value_error_new(const char *);
extern value_t *value_error_newf(const char *, ...);
extern value_t *value_int_new(int);
extern value_t *value_bif_new(void *, unsigned int);
extern value_t *value_udf_new(ast_t *, ast_t *);
extern void value_free(value_t *);
extern void value_print(value_t *, FILE *);

#define VALUE_ERR_MSG(e) ((e)->u.err_val.msg->text)
#define VALUE_FUNC_ISBIF(f) ((f)->u.func_val.is_bif)
#define VALUE_BIF_ARITY(f) ((f)->u.func_val.arity)
#define VALUE_BIF_PTR(f) ((f)->u.func_val.u.bif_ptr)
#define VALUE_UDF_PARS(f) ((f)->u.func_val.u.udf.pars)
#define VALUE_UDF_BODY(f) ((f)->u.func_val.u.udf.body)
#define VALUE_INT_VALUE(i) ((i)->u.int_val)

#ifdef __cplusplus
}
#endif

#endif /* VALUE_H_ */
