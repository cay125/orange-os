#include <algorithm>

#include "lib/types.h"

int sys_resolve_init_array(int ret, function_t* init_array_start, function_t* init_array_end) {
  if (ret == 0 && init_array_start && init_array_end) {
    std::for_each(init_array_start,
                  init_array_end,
                  [](function_t fp) { fp(); });
  }
  return ret;
}
