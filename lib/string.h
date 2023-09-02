#ifndef LIB_STRING_H
#define LIB_STRING_H

#include "lib/types.h"

extern "C" {

void* memset(void* s, int v, size_t size);
void* memcpy(void *dest, const void *src, size_t n);
void* memmove(void *dest, const void *src, size_t n);
int memcmp(const void * ptr1, const void * ptr2, size_t num);
size_t strlen(const char* s);
int strcmp(const char *s1, const char *s2);
const char* strchr(const char* str,char c);

}

#endif