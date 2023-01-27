#include "kernel/trap.h"

#include "arch/riscv_reg.h"
#include "kernel/syscalls/syscall.h"
#include "kernel/utils.h"

extern "C" {
void exception_table();
void restore_user_context(uint64_t tvec_addr, bool inc_epc);
}

namespace kernel {

void ProcessTrap() {
  if (riscv::regs::mcause.is_interrupt()) {
    panic();
  }
  riscv::Exception e_code = riscv::regs::mcause.get_exception();
  switch (e_code) {
  case riscv::Exception::environment_call_from_u_mode:
    ProcessSystemCall();
    break;
  
  default:
    panic();
    break;
  }
  restore_user_context((uint64_t)exception_table, true);
}

}  // namespace kernel