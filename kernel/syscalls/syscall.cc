#include "kernel/syscalls/syscall.h"

#include "arch/riscv_reg.h"
#include "kernel/regs_frame.hpp"
#include "kernel/scheduler.h"
#include "kernel/syscalls/manager.h"

namespace kernel {

void ProcessSystemCall() {
  RegFrame* frame = Schedueler::Instance()->ThisProcess()->frame;
  frame->a0 = syscall::Manager::Instance()->DoWork(frame->a7);
}

}