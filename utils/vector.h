#ifndef UTILS_VECTOR_H_
#define UTILS_VECTOR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __vector_t vector_t;

typedef int (*ele_comp_t)(intptr_t, intptr_t);

struct __vector_t {
    intptr_t *slots;
    size_t capacity;
    size_t count;
};

extern vector_t *vector_new(void);
extern void vector_free(vector_t *);
extern void vector_push(vector_t *, intptr_t);
extern intptr_t vector_pop(vector_t *);
extern intptr_t vector_ref(vector_t *, size_t);
extern int vector_posif(vector_t *, intptr_t, ele_comp_t);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_VECTOR_H_ */
