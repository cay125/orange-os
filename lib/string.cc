#include "lib/string.h"

void* memset(void* s, int v, size_t size) {
  uint8_t* c = reinterpret_cast<uint8_t*>(s);
  for (size_t i = 0; i < size; ++i) {
    c[i] = v;
  }
  return s;
}