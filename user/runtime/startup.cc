#include <algorithm>
#include "lib/syscall.h"

using function_t = void(*)(void);

extern int main(int, char**);

extern function_t __init_array_start[];
extern function_t __init_array_end[];
extern function_t __fini_array_start[];
extern function_t __fini_array_end[];

extern "C" void __start__(int argc, char** argv);

void __start__(int argc, char** argv) {
  asm volatile  (
    ".option push;"
    ".option norelax;"
    "la gp, __global_pointer$;"
    ".option pop;"
  );
  std::for_each(__init_array_start,
                __init_array_end,
                [](function_t fp) { fp(); });
  int exit_code = main(argc, argv);
  std::for_each(__fini_array_start,
                __fini_array_end,
                [](function_t fp) { fp(); });
  syscall::exit(exit_code);
}
