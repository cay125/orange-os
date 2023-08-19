#include "kernel/utils.h"

#include "lib/printk.h"
#include "kernel/printf.h"
#include "kernel/utils.h"

namespace kernel {

__attribute__ ((aligned (4)))
void panic(const char* fmt, ...) {
  global_interrunpt_off();
  printf("!! kernel panic !!, reason: ");
  if (fmt == nullptr) {
    printf("No extra info\n");
  } else {
    va_list va;
    va_start(va, fmt);
    char out_str[128] = {0};
    char* p = out_str;
    simple_vsprintf(&p, fmt, va, nullptr);
    va_end(va);
    printf(out_str);
    printf("\n");
  }
  for (;;);
}

}  // namespace kernel