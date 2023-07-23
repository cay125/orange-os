#include "lib/assert.h"
#include "lib/string.h"

#include "kernel/utils.h"

extern "C" {

void __assert_fail (
  const char *__assertion,
  const char *__file,
  unsigned int __line,
  const char *__function) {
  kernel::panic(__assertion);
}


}
