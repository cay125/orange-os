#include "kernel/trap.h"

#include "arch/riscv_reg.h"
#include "kernel/syscalls/syscall.h"
#include "kernel/utils.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"

namespace kernel {

void ProcessException() {
  riscv::Exception e_code = riscv::regs::mcause.get_exception();
  switch (e_code) {
  case riscv::Exception::environment_call_from_u_mode:
    ProcessSystemCall();
    break;
  
  default:
    panic();
    break;
  }
  Schedueler::Instance()->ThisProcess()->frame->mepc += 4;
  TrapRet(Schedueler::Instance()->ThisProcess(), e_code);
}

void ProcessInterrupt() {
  riscv::Interrupt i_code = riscv::regs::mcause.get_interrupt();
  switch (i_code) {
  case riscv::Interrupt::m_timer:
    Schedueler::Instance()->Yield();
    break;
  
  default:
    panic();
    break;
  }
  TrapRet(Schedueler::Instance()->ThisProcess(), riscv::Exception::none);
}

void ProcessTrap() {
  if (riscv::regs::mcause.is_interrupt()) {
    ProcessInterrupt();
    return;
  }
  if (riscv::regs::mcause.is_exception()) {
    ProcessException();
    return;
  }
}

}  // namespace kernel
