#include <algorithm>

#include "lib/common.h"
#include "lib/string.h"

namespace lib {
namespace common {

int SplitString(const char* str, char vec[][32], char dim) {
  int split_level = 0;
  const int max_level = 16;
  while (true) {
    while (*str && *str == dim) {
      ++str;
    }
    if (!*str) {
      break;
    }
    if (split_level == max_level) {
      return -1;
    }
    const char* pos = str;
    while (*str && *str != dim) {
      ++str;
    }
    if ((str - pos + 1) > 32) {
      return -1;
    }
    std::copy(pos, str, vec[split_level]);
    vec[split_level][str - pos] = '\0';
    split_level += 1;
  }
  return split_level;
}

}  // namespace common
}  // namespace lib
