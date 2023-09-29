#ifndef KERNEL_SYSCALLS_UTILS_H
#define KERNEL_SYSCALLS_UTILS_H

#include <type_traits>

#include "kernel/scheduler.h"
#include "kernel/virtual_memory.h"
#include "lib/types.h"

namespace kernel {
namespace syscall {
namespace comm {

uint64_t GetRawArg(int order);
void SetRawArg(int order, uint64_t value);
int GetIntArg(int order);
void SetIntArg(int order, int value);
void GetStrArg(int order, char* buf, size_t size);

template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T GetIntegralArg(int order) {
  return static_cast<T>(GetRawArg(order));
}

template <typename T>
T* GetAddrArg(int order) {
  uint64_t user_addr = GetRawArg(order);
  if (!user_addr) {
    return nullptr;
  }
  auto* process = Schedueler::Instance()->ThisProcess();
  riscv::PTE pte = riscv::PTE::None;
  auto pa = VirtualMemory::Instance()->VAToPA(process->page_table, user_addr, &pte);
  if (!pa || !(pte & riscv::PTE::U)) {
    return nullptr;
  }
  return reinterpret_cast<T*>(pa);
}

}  // namespace comm
}  // namespace syscall
}  // namespace kernel

#endif  // KERNEL_SYSCALLS_UTILS_H