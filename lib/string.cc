#include "lib/string.h"

extern "C" {

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

void * memmove(void *dest, const void *src, size_t size) {
  const uint8_t* s = reinterpret_cast<const uint8_t*>(src);
  uint8_t* d = reinterpret_cast<uint8_t*>(dest);
  for (int i = size - 1; i >= 0; --i) {
    d[i] = s[i];
  }
  return dest;
}

size_t strlen(const char* s) {
  const char* p = s;
  while (*p) {
    p++;
  }
  return p - s;
}

}
