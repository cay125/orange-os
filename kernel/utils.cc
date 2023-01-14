#include "kernel/utils.h"

#include "kernel/printf.h"

namespace kernel {

void panic() {
  printf("!! kernel panic !!\n");
  for (;;);
}

}  // namespace kernel