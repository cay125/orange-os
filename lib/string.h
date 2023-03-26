#ifndef LIB_STRING_H
#define LIB_STRING_H

#include "lib/types.h"

extern "C" {

void* memset(void* s, int v, size_t size);
void* memcpy(void *dest, const void *src, size_t n);
void* memmove(void *dest, const void *src, size_t n);
size_t strlen(const char* s);
int strcmp(const char *s1, const char *s2);

}

#endif