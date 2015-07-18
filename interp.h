#ifndef INTERP_H_
#define INTERP_H_

#include "ast.h"
#include "env.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __interp_t interp_t;

struct __interp_t {
    env_t *env;
};

extern interp_t *interp_new(void);
extern void interp_free(interp_t *);
extern value_kind_t interp_execute(interp_t *, ast_t *, value_t **);

#ifdef __cplusplus
}
#endif

#endif /* INTERP_H_ */
