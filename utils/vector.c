#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

/* PRIVATE */

static int vector_isfull(vector_t *v)
{
    return v->count + 1 >= v->capacity;
}

static void vector_incr(vector_t *v, size_t d)
{
    v->capacity += d;
    v->slots = realloc(v->slots, v->capacity * sizeof(intptr_t));
}

/* PUBLIC */

vector_t *vector_new(void)
{
    vector_t *v = malloc(sizeof(vector_t));
    v->slots = NULL;
    v->capacity = 0;
    v->count = 0;
    return v;
}

void vector_free(vector_t *v)
{
    free(v->slots);
    free(v);
}

void vector_push(vector_t *v, intptr_t data)
{
#define DELTA 10
    if (vector_isfull(v))
        vector_incr(v, DELTA);
    v->slots[v->count] = data;
    v->count++;
#undef DELTA
}

intptr_t vector_pop(vector_t *v)
{
    v->count--;
    return v->slots[v->count];
}

intptr_t vector_ref(vector_t *v, size_t index)
{
    return v->slots[index];
}

intptr_t vector_iref(vector_t *v, int index)
{
    return v->slots[v->count - 1 - index];
}

int vector_posif(vector_t *v, intptr_t key, ele_comp_t comp_func)
{
    int i = 0;
    while (i < v->count) {
        if ((*comp_func)(v->slots[i], key))
            return i;
        i++;
    }
    return -1;
}

intptr_t vector_top(vector_t *v)
{
    return v->slots[v->count - 1];
}

void vector_set(vector_t *v, intptr_t obj, int index)
{
    v->slots[index] = obj;
}

void vector_shrink(vector_t *v, size_t n)
{
    v->count -= n;
    if (v->count < 0)
        v->count = 0;
}

size_t vector_curpos(vector_t *v)
{
    return v->count;
}

void vector_setpos(vector_t *v, size_t pos)
{
    v->count = pos;
}
