#include "kernel/syscalls/syscall.h"

#include "arch/riscv_reg.h"
#include "kernel/regs_frame.hpp"
#include "kernel/scheduler.h"
#include "kernel/syscalls/manager.h"

namespace kernel {

void ProcessSystemCall() {
  ProcessTask* process = Schedueler::Instance()->ThisProcess();
  process->frame->a0 = syscall::Manager::Instance()->DoWork(process->frame->a7);
}

}