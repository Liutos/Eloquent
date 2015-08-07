#ifndef ENV_H_
#define ENV_H_

#include "utils/hash_table.h"
#include "utils/vector.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __env_t env_t;

struct __env_t {
    vector_t data;
    env_t *outer;
};

extern env_t *env_empty_new(void);
extern env_t *env_new(env_t *);
extern void env_free(env_t *);
extern void env_push(env_t *, const char *, value_t *, int *, int *);
extern void env_set(env_t *, const char *, value_t *);
extern int env_isempty(env_t *);
extern int env_locate(env_t *, const char *, int *, int *);
extern int env_update(env_t *, int, int, value_t *);
extern value_t *env_get(env_t *, const char *);
extern value_t *env_ref(env_t *, int, int);

#ifdef __cplusplus
}
#endif

#endif /* ENV_H_ */
