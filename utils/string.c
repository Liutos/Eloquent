/*
 * string.c
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#include <stdio.h>
#include <stdlib.h>

#include "string.h"

/* PRIVATE */

static int string_isfull(string_t *s)
{
    return s->length + 1 >= s->capacity;
}

static void string_incr(string_t *s, size_t d)
{
    s->capacity += d;
    s->text = realloc(s->text, s->capacity);
}

/* PUBLIC */

string_t *string_new(void)
{
    string_t *s = malloc(sizeof(string_t));
    s->capacity = 0;
    s->length = 0;
    s->text = NULL;
    return s;
}

void string_free(string_t *s)
{
    free(s->text);
    free(s);
}

void string_addc(string_t *s, char c)
{
#define DELTA 10
    if (string_isfull(s))
        string_incr(s, DELTA);
    s->text[s->length] = c;
    s->length++;
#undef DELTA
}

void string_assign(string_t *str, const char *src)
{
    while (*src != '\0') {
        string_addc(str, *src);
        src++;
    }
}

void string_clear(string_t *s)
{
    s->length = 0;
}
