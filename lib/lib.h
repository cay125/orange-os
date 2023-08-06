#ifndef LIB_LIB_H
#define LIB_LIB_H

#include "lib/types.h"
#include "lib/syscall.h"

enum class PrintLevel {
  info,
  error,
};

// user functions
void printf(const char *fmt, ...);
void printf(PrintLevel level, const char *fmt, ...);

#endif  // LIB_LIB_H