#include "lib/string.h"

void* memset(void* s, int v, size_t size) {
  uint8_t* c = reinterpret_cast<uint8_t*>(s);
  for (size_t i = 0; i < size; ++i) {
    c[i] = v;
  }
  return s;
}

void* memcpy(void *dest, const void *src, size_t n) {
  const uint8_t* s = reinterpret_cast<const uint8_t*>(src);
  uint8_t* d = reinterpret_cast<uint8_t*>(dest);
  for (size_t i = 0; i < n; ++i) {
    d[i] = s[i];
  }
  return dest;
}