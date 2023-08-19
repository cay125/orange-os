#ifndef KERNEL_REGS_FRSME_H
#define KERNEL_REGS_FRSME_H

#include "arch/riscv_isa.h"
#include "lib/types.h"

namespace kernel {

struct RegFrame {
  uint64_t ra;
  uint64_t sp;
  uint64_t gp;
  uint64_t tp;
  uint64_t t0;
  uint64_t t1;
  uint64_t t2;
  uint64_t s0;
  uint64_t s1;
  uint64_t a0;
  uint64_t a1;
  uint64_t a2;
  uint64_t a3;
  uint64_t a4;
  uint64_t a5;
  uint64_t a6;
  uint64_t a7;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
  uint64_t t3;
  uint64_t t4;
  uint64_t t5;
  uint64_t t6;
  uint64_t mepc;
  uint64_t mcause;
  uint64_t kernel_sp;
  uint64_t reserved_space[128 - 34];

  uint64_t temporary_space[128];

  riscv::Interrupt interrunpt() {
    return static_cast<riscv::Interrupt>(mcause & ((1 << (8 * sizeof(riscv::Interrupt))) - 1));
  }

  riscv::Exception exception() {
    return static_cast<riscv::Exception>(mcause & ((1 << (8 * sizeof(riscv::Interrupt))) - 1));
  }

};

}  // namespace kernel

#endif