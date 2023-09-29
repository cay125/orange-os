#include <algorithm>
#include <elf.h>

#include "arch/riscv_isa.h"
#include "arch/riscv_reg.h"
#include "kernel/config/memory_layout.h"
#include "kernel/printf.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/static_loader.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/types.h"
#include "lib/string.h"
#include "lib/streambase.h"
#include "lib/stringstream.h"

extern "C" {
void restore_user_context(kernel::RegFrame* frame);
void user_exception_table();
}

namespace kernel {

int ExecuteImpl(lib::StreamBase* stream, ProcessTask* process) {
  uint64_t entry;
  StaticLoader static_loader(stream, process, 0);
  if (!static_loader.Load(ET_EXEC, &entry, nullptr)) {
    return -1;
  }
  process->frame->mepc = entry;
  return 0;
}

void TrapRet(ProcessTask* process, riscv::Exception exception) {
  global_interrunpt_off();
  RegFrame* frame = process->frame;
  riscv::regs::satp.write(riscv::virtual_addresing::Sv39, process->page_table);
  riscv::isa::sfence();
  riscv::regs::mepc.write(frame->mepc);
  riscv::regs::mtvec.write_vec(user_exception_table, true);
  riscv::regs::mstatus.set_mpp(riscv::MPP::user_mode);
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mpie);
  restore_user_context(frame);
}

void ExecuteRet() {
  auto* process = Schedueler::Instance()->ThisProcess();
  process->lock.UnLock();
  TrapRet(process, riscv::Exception::none);
}

void ExcuteInitProcess(char* code, size_t size) {
  lib::StringStream stringstream(code, size);
  ProcessTask* process = Schedueler::Instance()->AllocProc();
  ExecuteImpl(&stringstream, process);
  process->state = ProcessState::runnable;
  Schedueler::Instance()->SetInitProcess(process);
}

}  // namespace kernel