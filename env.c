#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "utils/vector.h"

typedef struct __binding_t binding_t;

struct __binding_t {
    const char *name;
    void *value;
};

/* PRIVATE */

static binding_t *binding_new(const char *name, void *value)
{
    binding_t *b = malloc(sizeof(binding_t));
    b->name = name;
    b->value = value;
    return b;
}

/* PUBLIC */

env_t *env_empty_new(void)
{
    return NULL;
}

env_t *env_new(env_t *outer)
{
    env_t *env = malloc(sizeof(env_t));
    vector_init(&env->data);
    env->outer = outer;
    return env;
}

void env_free(env_t *env)
{
    vector_free(&env->data);
    free(env);
}

void env_set(env_t *env, const char *name, value_t *value)
{
    for (int i = 0; i < env->data.count; i++) {
        binding_t *b = (binding_t *)vector_ref(&env->data, i);
        if (strcmp(b->name, name) == 0) {
            b->value = value;
            return;
        }
    }
    binding_t *b = binding_new(name, value);
    vector_push(&env->data, (intptr_t)b);
}

int env_isempty(env_t *env)
{
    return env == NULL;
}

value_t *env_get(env_t *env, const char *name)
{
    while (!env_isempty(env)) {
        for (int i = 0; i < env->data.count; i++) {
            binding_t *b = (binding_t *)vector_ref(&env->data, i);
            if (strcmp(b->name, name) == 0)
                return b->value;
        }
        env = env->outer;
    }
    return NULL;
}
