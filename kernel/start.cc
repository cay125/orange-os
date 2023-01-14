#include "kernel/printf.h"
#include "lib/types.h"

__attribute__ ((aligned (16))) char stack[4096];

extern void (*init_array_start[])(void);
extern void (*init_array_end[])(void);

namespace kernel {

void RunInitArray() {
  int count = init_array_end - init_array_start;
  for (int i = 0; i < count; ++i) {
    uint64_t addr = reinterpret_cast<uint64_t>(init_array_start[i]);
    if (addr == 0 || addr == 0xffffffffffffffff) {
      continue;
    }
    init_array_start[i]();
  }
}

}  // namespace kernel

void start() {
  kernel::RunInitArray();
}
