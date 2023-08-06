#include <algorithm>

#include "lib/common.h"
#include "lib/string.h"

namespace lib {
namespace common {

int SplitString(const char* str, char vec[][32], char dim) {
  int split_level = 0;
  const int max_level = 16;
  const char* paths[max_level] = {nullptr};
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
    paths[split_level++] = str;
    while (*str && *str != dim) {
      ++str;
    }
  }
  for (auto i = 0; i < split_level; i++) {
    size_t len = (i < (split_level - 1)) ? paths[i + 1] - paths[i] : strlen(paths[i]);
    if (len >= 32) {
      return -1;
    }
    std::copy(paths[i], paths[i] + len, vec[i]);
    vec[i][len] = '\0';
  }
  return split_level;
}

}  // namespace common
}  // namespace lib
