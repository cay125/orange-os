#ifndef KERNEL_SYSCALLS_UTILS_H
#define KERNEL_SYSCALLS_UTILS_H

#include <type_traits>

#include "lib/types.h"

namespace kernel {
namespace syscall {
namespace comm {

uint64_t GetRawArg(int order);
void SetRawArg(int order, uint64_t value);
int GetIntArg(int order);
void SetIntArg(int order, int value);

template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T GetIntegralArg(int order) {
  return static_cast<T>(GetRawArg(order));
}

template <typename T>
T* GetAddrArg(int order) {
  return reinterpret_cast<T*>(GetRawArg(order));
}

}  // namespace comm
}  // namespace syscall
}  // namespace kernel

#endif  // KERNEL_SYSCALLS_UTILS_H