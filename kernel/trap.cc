#include "kernel/trap.h"

#include "arch/riscv_reg.h"
#include "kernel/extern_controller.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/syscall.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/common.h"

namespace kernel {

void ProcessException() {
  ProcessTask* process = Schedueler::Instance()->ThisProcess();
  riscv::Exception e_code = process->frame->exception();
  switch (e_code) {
  case riscv::Exception::environment_call_from_u_mode:
    global_interrunpt_on();
    process->frame->mepc += 4;
    ProcessSystemCall();
    break;
  
  case riscv::Exception::store_page_fault:
  case riscv::Exception::load_page_fault:
    if (process->frame->sp < (VirtualMemory::GetUserSpVa() - memory_layout::PGSIZE)) {
      printf("Stack overflow: %p vs %p\n", process->frame->sp, VirtualMemory::GetUserSpVa() - memory_layout::PGSIZE);
    } else {
      printf("Segment fault, trying to %s invalid addr: 0x%p\n", e_code == riscv::Exception::store_page_fault ? "write" : "read", riscv::regs::mtval.read());
    }
    Schedueler::Instance()->Exit(-1);
    break;

  default:
    panic("Unexpted exception code: %d", lib::common::literal(e_code));
    break;
  }
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

  case riscv::Interrupt::m_external:
    ExternController::Instance()->DoWork();
    break;
  
  default:
    panic("Unexpectd interrunpt code: %d", lib::common::literal(i_code));
    break;
  }
  TrapRet(Schedueler::Instance()->ThisProcess(), riscv::Exception::none);
}

void ProcessUserTrap() {
  if (riscv::regs::mstatus.read_mpp() != riscv::MPP::user_mode) {
    panic("Invalid privilege mode, expected: user_mode");
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
    panic("Invalid privilege mode, expected: machine_mode");
  }
  riscv::Interrupt i_code = riscv::regs::mcause.get_interrupt();
  if (i_code == riscv::Interrupt::m_timer) {
    if (cpu_id() == 0) {
      Schedueler::Instance()->ClockInterrupt();
    }
    if (!Schedueler::Instance()->ThisCpu()->process_task) {
      return;
    }
    Schedueler::Instance()->Yield();
  } else if (i_code == riscv::Interrupt::m_external) {
    ExternController::Instance()->DoWork();
  }
  riscv::regs::mstatus.set_mpp(riscv::MPP::machine_mode);
  riscv::regs::mstatus.set_bit(riscv::StatusBit::mpie);
}

}  // namespace kernel
