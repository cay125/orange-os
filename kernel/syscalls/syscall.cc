#include "kernel/syscalls/syscall.h"

#include "arch/riscv_reg.h"
#include "kernel/regs_frame.hpp"
#include "kernel/syscalls/manager.h"

namespace kernel {

void ProcessSystemCall() {
  RegFrame* frame = reinterpret_cast<RegFrame*>(riscv::regs::mscratch.read());
  frame->a0 = syscall::Manager::Instance()->DoWork(frame->a7);
}

}