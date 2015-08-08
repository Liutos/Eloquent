#include <stdio.h>
#include <stdlib.h>

#include "ident.h"
#include "utils/hash_table.h"

static hash_table_t *ident_table = NULL;

static hash_table_t *ident_table_new(void)
{
    return hash_table_new(hash_str, comp_str);
}

static ident_t *ident_new(const char *name)
{
    ident_t *id = malloc(sizeof(ident_t));
    string_init(&id->name);
    string_assign(&id->name, name);
    return id;
}

/* PUBLIC */

ident_t *ident_intern(const char *name)
{
    if (ident_table == NULL)
        ident_table = ident_table_new();
    ident_t *id = hash_table_get(ident_table, (void *)name, NULL);
    if (id == NULL) {
        id = ident_new(name);
        hash_table_set(ident_table, (void *)name, id);
    }
    return id;
}
