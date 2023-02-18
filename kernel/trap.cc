#include "kernel/trap.h"

#include "arch/riscv_reg.h"
#include "kernel/syscalls/syscall.h"
#include "kernel/utils.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"

namespace kernel {

void ProcessException() {
  riscv::Exception e_code = Schedueler::Instance()->ThisProcess()->frame->exception();
  switch (e_code) {
  case riscv::Exception::environment_call_from_u_mode:
    global_interrunpt_on();
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
  riscv::Interrupt i_code = Schedueler::Instance()->ThisProcess()->frame->interrunpt();
  switch (i_code) {
  case riscv::Interrupt::m_timer:
    if (cpu_id() == 0) {
      Schedueler::Instance()->ClockInterrupt();
    }
    Schedueler::Instance()->Yield();
    break;
  
  default:
    panic();
    break;
  }
  TrapRet(Schedueler::Instance()->ThisProcess(), riscv::Exception::none);
}

void ProcessUserTrap() {
  if (riscv::regs::mstatus.read_mpp() != riscv::MPP::user_mode) {
    panic();
  }
  auto* process = Schedueler::Instance()->ThisProcess();
  if (process->frame->mcause & riscv::EXCEPTION_MASK) {
    ProcessInterrupt();
    return;
  }
  if (!(process->frame->mcause & riscv::EXCEPTION_MASK)) {
    ProcessException();
    return;
  }
}

void ProcessKernelTrap() {
  if (riscv::regs::mstatus.read_mpp() != riscv::MPP::machine_mode) {
    panic();
  }
  if (cpu_id() == 0) {
    Schedueler::Instance()->ClockInterrupt();
  }
  if (!Schedueler::Instance()->ThisCpu()->process_task) {
    return;
  }
  Schedueler::Instance()->Yield();
  auto* process = Schedueler::Instance()->ThisProcess();
  riscv::regs::satp.write(riscv::virtual_addresing::Sv39, process->page_table);
  riscv::isa::sfence();
  riscv::regs::mstatus.set_mpp(riscv::MPP::machine_mode);
  riscv::regs::mstatus.set_bit(riscv::StatusBit::mpie);
}

}  // namespace kernel
