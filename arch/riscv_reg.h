#ifndef ARCH_RISCV_REG_H
#define ARCH_RISCV_REG_H

namespace riscv {
namespace reg {

struct a7_op {
  template <int imm>
  static void write_imm() {
    asm volatile ("li a7, %0" : : "i" (imm));
  }
};

template <class OP>
class WriteReg {
 public:
  template <int imm>
  inline void write_imm() {
    OP::template write_imm<imm>();
  }
};

using a7_reg = WriteReg<a7_op>;

struct instances {
  a7_reg a7;
};

}  // namespace reg

static reg::instances regs;

}  // namespace riscv

#endif  // ARCH_RISCV_REG_H