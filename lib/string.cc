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

int memcmp(const void * ptr1, const void * ptr2, size_t num) {
  const uint8_t* s = reinterpret_cast<const uint8_t*>(ptr1);
  const uint8_t* d = reinterpret_cast<const uint8_t*>(ptr2);
  for (size_t i = 0; i < num; i++) {
    if (s > d) {
      return 1;
    }
    if (s < d) {
      return -1;
    }
  }
  return 0;
}

size_t strlen(const char* s) {
  const char* p = s;
  while (*p) {
    p++;
  }
  return p - s;
}

int strcmp(const char *s1, const char *s2) {
  auto len1 = strlen(s1);
  auto len2 = strlen(s2);
  if (len1 > len2) {
    return 1;
  } else if (len1 < len2) {
    return -1;
  }
  for (size_t i = 0; i < len1; ++i) {
    if (s1[i] > s2[i]) {
      return 1;
    }
    if (s1[i] < s2[i]) {
      return -1;
    }
  }
  return 0;
}

const char* strchr(const char* str, char c) {
  while (*str) {
    if (*str == c) {
      return str;
    }
    str++;
  }
  return nullptr;
}

}
