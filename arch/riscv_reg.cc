#include "arch/riscv_reg.h"

namespace riscv {

// registers
details::a7_reg regs::a7;

// CSRs
details::mtvec regs::mtvec;
details::mepc regs::mepc;
details::mstatus regs::mstatus;
details::mscratch regs::mscratch;
details::mcause regs::mcause;
details::mhartid regs::mhartid;
details::mie regs::mie;
details::satp regs::satp;

details::pmp_addr regs::pmp_addr;
details::pmp_cfg regs::pmp_cfg;

}  // namespace riscv