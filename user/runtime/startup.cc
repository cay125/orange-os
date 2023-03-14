#include <algorithm>

using function_t = void(*)(void);

extern int main(void);

extern function_t __init_array_start[];
extern function_t __init_array_end[];
extern function_t __fini_array_start[];
extern function_t __fini_array_end[];

extern "C" void _start();

void _start() {
  asm volatile  (
    ".option push;"
    ".option norelax;"
    "la gp, __global_pointer$;"
    ".option pop;"
  );
  std::for_each(__init_array_start,
                __init_array_end,
                [](function_t fp) { fp(); });
  main();
  std::for_each(__fini_array_start,
                __fini_array_end,
                [](function_t fp) { fp(); });
}
