#include "kernel/utils.h"

#include "kernel/printf.h"

namespace kernel {

__attribute__ ((aligned (4)))
void panic() {
  printf("!! kernel panic !!\n");
  for (;;);
}

}  // namespace kernel