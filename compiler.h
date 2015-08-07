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
#include "env.h"
#include "utils/hash_table.h"
#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __compiler_t compiler_t;

struct __compiler_t {
    unsigned int counter;
    env_t *env;
    hash_table_t *label_table;
    hash_table_t *rts;
    string_t *error;
};

extern compiler_t *compiler_new(void);
extern void compiler_free(compiler_t *);
extern int compiler_do(compiler_t *, ast_t *, ins_t *);

#ifdef __cplusplus
}
#endif

#endif /* COMPILER_H_ */
