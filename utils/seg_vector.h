/*
 * seg_vector.h
 *
 *  Created on: 2015年7月22日
 *      Author: liutos
 */

#ifndef UTILS_SEG_VECTOR_H_
#define UTILS_SEG_VECTOR_H_

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __seg_vector_t seg_vector_t;

struct __seg_vector_t {
    vector_t *data;
    seg_vector_t *next;
};

extern seg_vector_t *seg_vector_new(seg_vector_t *);
extern void seg_vector_free(seg_vector_t *);
extern void seg_vector_push(seg_vector_t *, const void *);
extern int seg_vector_locate(seg_vector_t *, const void *, ele_comp_t, int *, int *);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_SEG_VECTOR_H_ */
