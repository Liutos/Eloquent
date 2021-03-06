#ifndef INTERP_H_
#define INTERP_H_

#include "ast.h"
#include "env.h"
#include "utils/hash_table.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __interp_t interp_t;
typedef struct __syntax_t syntax_t;

typedef value_kind_t (*bis_t)(interp_t *, ast_t *, value_t **);

struct __syntax_t {
    void *ptr;
};

struct __interp_t {
    int depth;
    hash_table_t *syntax_env;
    hash_table_t *traced_objs;
    env_t *env;
    env_t *denv;
};

extern interp_t *interp_new(void);
extern value_kind_t interp_execute(interp_t *, ast_t *, value_t **);

#ifdef __cplusplus
}
#endif

#endif /* INTERP_H_ */
