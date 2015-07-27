#ifndef ENV_H_
#define ENV_H_

#include "utils/hash_table.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __env_t env_t;

struct __env_t {
    hash_table_t *data;
    env_t *outer;
};

extern env_t *env_new(env_t *);
extern env_t *env_empty_new(void);
extern int env_isempty(env_t *);
extern void env_free(env_t *);
extern value_t *env_get(env_t *, char *, int *);
extern void env_set(env_t *, const char *, value_t *);

#ifdef __cplusplus
}
#endif

#endif /* ENV_H_ */
