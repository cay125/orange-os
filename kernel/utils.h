#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include <algorithm>
#include <type_traits>

#include "arch/riscv_reg.h"
#include "kernel/config/memory_layout.h"
#include "kernel/scheduler.h"
#include "kernel/virtual_memory.h"

namespace kernel {

void panic(const char* fmt = nullptr, ...);

template <typename T, int N>
int Len(T (&arr)[N]) {
  return N;
}

inline __attribute__((always_inline)) uint64_t cpu_id() {
  return riscv::regs::tp.read();
}

inline __attribute__((always_inline)) void global_interrunpt_on() {
  riscv::regs::mstatus.set_bit(riscv::StatusBit::mie);
}

inline __attribute__((always_inline)) void global_interrunpt_off() {
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mie);
}

template <typename F, typename = std::enable_if_t<std::is_invocable_v<F&, char*, size_t>>>
size_t safe_copy(uint64_t dst, size_t size, F& fun) {
  auto* process = Schedueler::Instance()->ThisProcess();
  auto f = [process](uint64_t addr){
    return VirtualMemory::Instance()->VAToPA(process->page_table, addr);
  };
  size_t copyout_size = 0;
  while (size > 0) {
    uint64_t pa = f(dst);
    uint64_t remain = memory_layout::PGSIZE - pa % memory_layout::PGSIZE;
    size_t len = std::min(remain, size);
    size_t ret = fun(reinterpret_cast<char*>(pa), len);
    copyout_size += ret;
    if (ret < len) {
      return copyout_size;
    }
    dst += len;
    size -= len;
  }
  return copyout_size;
}

}  // namespace kernel

#endif  // KERNEL_UTILS_H