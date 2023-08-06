#ifndef LIB_COMMON_H
#define LIB_COMMON_H

#include <type_traits>

namespace lib {
namespace common {

template <class T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
inline __attribute__((always_inline)) std::underlying_type_t<T> literal(T n) {
  return static_cast<std::underlying_type_t<T>>(n);
}

int SplitString(const char* str, char vec[][32], char dim);

}  // namespace common
}  // namespace lib

#endif  // LIB_COMMON_H
