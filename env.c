#include <stdlib.h>
#include "env.h"
#include "utils/hash_table.h"

/* PUBLIC */

env_t *env_new(env_t *outer)
{
    env_t *env = malloc(sizeof(env_t));
    env->data = hash_table_new(hash_str, comp_str);
    env->outer = outer;
    return env;
}

void env_free(env_t *env)
{
    hash_table_free(env->data);
    free(env);
}

env_t *env_empty_new(void)
{
    return NULL;
}

int env_isempty(env_t *env)
{
    return env == NULL;
}

value_t *env_get(env_t *env, char *name, int *is_found)
{
    while (!env_isempty(env)) {
        value_t *val = hash_table_get(env->data, name, is_found);
        if (val != NULL) {
            if (is_found != NULL)
                *is_found = 1;
            return val;
        }
        env = env->outer;
    }
    if (is_found != NULL)
        *is_found = 0;
    return NULL;
}

void env_set(env_t *env, char *name, value_t *value)
{
    hash_table_set(env->data, name, value);
}
