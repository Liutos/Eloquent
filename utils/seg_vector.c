/*
 * seg_vector.c
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */
#include <stdlib.h>
#include "seg_vector.h"
#include "vector.h"

/* PUBLIC */

seg_vector_t *seg_vector_new(seg_vector_t *next)
{
    seg_vector_t *sv = malloc(sizeof(*sv));
    sv->data = vector_new();
    sv->next = next;
    return sv;
}

void seg_vector_free(seg_vector_t *sv)
{
    vector_free(sv->data);
    free(sv);
}

void seg_vector_push(seg_vector_t *sv, const void *ptr)
{
    vector_push(sv->data, (intptr_t)ptr);
}

int seg_vector_locate(seg_vector_t *sv, const void *key, ele_comp_t comp_func, int *seg_index, int *vector_index)
{
    int si = 0;
    while (sv != NULL) {
        int vi = vector_posif(sv->data, (intptr_t)key, comp_func);
        if (vi != -1) {
            if (seg_index != NULL)
                *seg_index = si;
            if (vector_index != NULL)
                *vector_index = vi;
            return 1;
        }
        sv = sv->next;
        si++;
    }
    return 0;
}

void *seg_vector_ref(seg_vector_t *sv, int i, int j)
{
    while (i > 0 && sv != NULL) {
        sv = sv->next;
        i--;
    }
    if (sv->data->count < j)
        return NULL;
    return (void *)vector_ref(sv->data, j);
}

int seg_vector_set(seg_vector_t *sv, void *obj, int i, int j)
{
    while (i > 0 && sv != NULL) {
        sv = sv->next;
        i--;
    }
    if (sv->data->count < j)
        return 0;
    vector_set(sv->data, (intptr_t)obj, j);
    return 1;
}
