#ifndef ARCH_RISCV_ISA_H
#define ARCH_RISCV_ISA_H

namespace riscv {
namespace isa {

inline void ecall() {
  asm volatile ("ecall");
}

inline void ret() {
  asm volatile ("ret");
}

}  // namespace isa
}  // namespace riscv

#endif  // ARCH_RISCV_ISA_H