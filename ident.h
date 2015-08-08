#ifndef IDENT_H_
#define IDENT_H_

#include "utils/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __ident_t ident_t;

struct __ident_t {
    string_t name;
};

extern ident_t *ident_intern(const char *);

#define IDENT_NAME(i) ((i)->name.text)

#ifdef __cplusplus
}
#endif

#endif /* IDENT_H_ */
