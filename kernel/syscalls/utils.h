#ifndef KERNEL_SYSCALLS_UTILS_H
#define KERNEL_SYSCALLS_UTILS_H

#include "lib/types.h"

namespace kernel {
namespace syscall {
namespace comm {

uint64_t GetRawArg(int order);
int GetIntArg(int order);

template <typename T>
T* GetAddrArg(int order) {
  return reinterpret_cast<T*>(GetRawArg(order));
}

}  // namespace comm
}  // namespace syscall
}  // namespace kernel

#endif  // KERNEL_SYSCALLS_UTILS_H