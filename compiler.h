/*
 * compiler.h
 *
 *  Created on: 2015年7月20日
 *      Author: liutos
 */

#ifndef COMPILER_H_
#define COMPILER_H_

#include "ast.h"
#include "bytecode.h"
#include "utils/hash_table.h"
#include "utils/seg_vector.h"
#include "utils/string.h"
#include "utils/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __compiler_t compiler_t;
typedef seg_vector_t compiler_env_t;

struct __compiler_t {
    string_t *error;
    hash_table_t *rts;
    compiler_env_t *env;
    unsigned int counter;
    hash_table_t *label_table;
};

extern compiler_t *compiler_new(void);
extern void compiler_free(compiler_t *);
extern int compiler_do(compiler_t *, ast_t *, ins_t *);

#ifdef __cplusplus
}
#endif

#endif /* COMPILER_H_ */
