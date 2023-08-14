#pragma once

#include "lib/types.h"

namespace lib {

void* malloc(uint32_t bytes);
void free(void* addr);

}  // namespace lib

void* operator new(unsigned long size);
void operator delete(void* addr, unsigned long);
