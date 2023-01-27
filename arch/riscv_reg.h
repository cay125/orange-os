#ifndef ARCH_RISCV_REG_H
#define ARCH_RISCV_REG_H

#include "arch/riscv_isa.h"
#include "lib/types.h"

namespace riscv {

struct regs;
namespace details {

struct a7_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("li a7, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("mv a7, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("ld %0, a7" : "=r" (v));
    return v;
  }
};

struct mtvec_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("csrwi mtvec, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("csrw mtvec, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mtvec" : "=r" (v));
    return v;
  }
};

struct mepc_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("csrwi mepc, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("csrw mepc, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mepc" : "=r" (v));
    return v;
  }
};

struct mstatus_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("csrwi mstatus, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("csrw mstatus, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mstatus" : "=r" (v));
    return v;
  }
};

struct mscratch_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("csrwi mscratch, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("csrw mscratch, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mscratch" : "=r" (v));
    return v;
  }
};

struct satp_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("csrwi satp, %0" : : "i" (imm));
  }

  static void write(uint64_t v) {
    asm volatile ("csrw satp, %0" : : "r" (v));
  }

  static uint64_t read() {
    uint64_t v = 0;
    asm volatile ("csrr %0, satp" : "=r" (v));
    return v;
  }
};

template <class OP>
class GeneralReg {
 public:
  GeneralReg(GeneralReg&) = delete;

  inline void write(uint64_t v) {
    OP::write(v);
  }

  template <int imm>
  inline void write_imm() {
    OP::template write_imm<imm>();
  }

  inline uint64_t read() {
    return OP::read();
  }

 private:
  GeneralReg() {}

  friend class ::riscv::regs;
};

class MstatusImpl{
 public:
  void set_mpp(MPP mpp) {
    uint64_t v = 0;
    asm volatile ("csrr %0, mstatus" : "=r" (v));
    v &= MPP_MASK;
    v |= mpp;
    asm volatile ("csrw mstatus, %0" : : "r" (v));
  }

  void set_bit(StatusBit bit) {
    uint64_t v = 0;
    asm volatile ("csrr %0, mstatus" : "=r" (v));
    v |= bit;
    asm volatile ("csrw mstatus, %0" : : "r" (v));
  }

  void clear_bit(StatusBit bit) {
    uint64_t v = 0;
    asm volatile ("csrr %0, mstatus" : "=r" (v));
    v &= ~bit;
    asm volatile ("csrw mstatus, %0" : : "r" (v));
  }

 private:
  MstatusImpl() {}

  friend class ::riscv::regs;
};

class PMPAddrImpl {
 public:
  void write(uint8_t index, uint64_t v) {
    if (index >= MAX_PMP_REG) return;
    switch (index) {
    case 0:
      asm volatile ("csrw pmpaddr0, %0" : : "r" (v));
      break;
    case 1:
      asm volatile ("csrw pmpaddr1, %0" : : "r" (v));
      break;
    case 2:
      asm volatile ("csrw pmpaddr2, %0" : : "r" (v));
      break;
    case 3:
      asm volatile ("csrw pmpaddr3, %0" : : "r" (v));
      break;
    case 4:
      asm volatile ("csrw pmpaddr4, %0" : : "r" (v));
      break;
    case 5:
      asm volatile ("csrw pmpaddr5, %0" : : "r" (v));
      break;
    case 6:
      asm volatile ("csrw pmpaddr6, %0" : : "r" (v));
      break;
    case 7:
      asm volatile ("csrw pmpaddr7, %0" : : "r" (v));
      break;
    case 8:
      asm volatile ("csrw pmpaddr8, %0" : : "r" (v));
      break;
    case 9:
      asm volatile ("csrw pmpaddr9, %0" : : "r" (v));
      break;
    case 10:
      asm volatile ("csrw pmpaddr10, %0" : : "r" (v));
      break;
    case 11:
      asm volatile ("csrw pmpaddr11, %0" : : "r" (v));
      break;
    case 12:
      asm volatile ("csrw pmpaddr12, %0" : : "r" (v));
      break;
    case 13:
      asm volatile ("csrw pmpaddr13, %0" : : "r" (v));
      break;
    case 14:
      asm volatile ("csrw pmpaddr14, %0" : : "r" (v));
      break;
    case 15:
      asm volatile ("csrw pmpaddr15, %0" : : "r" (v));
      break;
    default:
      break;
    }
  }
 private:
  PMPAddrImpl() {}

  friend class ::riscv::regs;
};

class PMPCfgImpl {
 public:
  void write(uint8_t index, PMPBit bit) {
    if (index >= MAX_PMP_REG) return;
    if (index < 8) {
      uint64_t v = bit << index;
      asm volatile ("csrw pmpcfg0, %0" : : "r" (v));
    } else {
      uint64_t v = bit << (index - 8);
      asm volatile ("csrw pmpcfg2, %0" : : "r" (v));
    }
  }

 private:
  PMPCfgImpl() {}

  friend class ::riscv::regs;
};

class McauseImpl {
 public:
  bool is_exception() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mcause" : "=r" (v));
    if (v & riscv::EXCEPTION_MASK) {
      return false;
    } else {
      return true;
    }
  }

  bool is_interrupt() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mcause" : "=r" (v));
    if (!(v & riscv::EXCEPTION_MASK)) {
      return false;
    } else {
      return true;
    }
  }

  riscv::Exception get_exception() {
    uint64_t v = 0;
    asm volatile ("csrr %0, mcause" : "=r" (v));
    return static_cast<riscv::Exception>(v);
  }

 private:
  McauseImpl() {}

  friend class ::riscv::regs;
};

using a7_reg = GeneralReg<a7_op>;

using mtvec = GeneralReg<mtvec_op>;
using mepc = GeneralReg<mepc_op>;
using mstatus = MstatusImpl;
using mscratch = GeneralReg<mscratch_op>;
using mcause = McauseImpl;

using satp = GeneralReg<satp_op>;

using pmp_addr = PMPAddrImpl;
using pmp_cfg = PMPCfgImpl;

}  // namespace details

struct regs {
  regs() = delete;
  regs(const regs&) = delete;

  // registers
  static details::a7_reg a7;

  // CSRs
  static details::mtvec mtvec;
  static details::mepc mepc;
  static details::mstatus mstatus;
  static details::mscratch mscratch;
  static details::mcause mcause;

  static details::satp satp;

  static details::pmp_addr pmp_addr;
  static details::pmp_cfg pmp_cfg;
};

}  // namespace riscv

#endif  // ARCH_RISCV_REG_H