#ifndef LIB_STRING_H
#define LIB_STRING_H

#include "lib/types.h"

void* memset(void* s, int v, size_t size);
void* memcpy(void *dest, const void *src, size_t n);
extern "C" void* memmove(void *dest, const void *src, size_t n);
size_t strlen(const char* s);

#endif