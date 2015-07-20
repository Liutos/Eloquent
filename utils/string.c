/*
 * string.c
 *
 *  Created on: 2015年7月16日
 *      Author: liutos
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "string.h"

#define DELTA 10

/* PRIVATE */

static int string_isfull(string_t *s)
{
    return s->length + 1 >= s->capacity;
}

static void string_incr(string_t *s, size_t d)
{
    s->capacity += d;
    s->text = realloc(s->text, s->capacity * sizeof(char));
}

/* PUBLIC */

string_t *string_new(void)
{
    string_t *s = malloc(sizeof(string_t));
    s->capacity = DELTA;
    s->length = 0;
    s->text = calloc(s->capacity, sizeof(char));
    return s;
}

void string_free(string_t *s)
{
    free(s->text);
    free(s);
}

void string_addc(string_t *s, char c)
{
    if (string_isfull(s))
        string_incr(s, DELTA);
    s->text[s->length] = c;
    s->length++;
}

void string_assign(string_t *str, const char *src)
{
    while (*src != '\0') {
        string_addc(str, *src);
        src++;
    }
    string_addc(str, '\0');
}

void string_clear(string_t *s)
{
    bzero(s->text, s->length);
    s->length = 0;
}

int string_printf(string_t *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(s->text, s->capacity - 1, fmt, ap);
    if (len >= s->capacity - 1) {
        s->capacity = len + DELTA;
        s->text = realloc(s->text, s->capacity * sizeof(char));
    }
    return vsnprintf(s->text, s->capacity - 1, fmt, ap);
}
