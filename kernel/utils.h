#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "arch/riscv_reg.h"

namespace kernel {

void panic();

inline __attribute__((always_inline)) void global_interrunpt_on() {
  riscv::regs::mstatus.set_bit(riscv::StatusBit::mie);
}

inline __attribute__((always_inline)) void global_interrunpt_off() {
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mie);
}

}  // namespace kernel

#endif  // KERNEL_UTILS_H