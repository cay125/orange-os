#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "arch/riscv_reg.h"

namespace kernel {

void panic();

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

}  // namespace kernel

#endif  // KERNEL_UTILS_H