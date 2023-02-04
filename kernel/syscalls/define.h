#ifndef KERNEL_SYSCALLS_DEFINE_H
#define KERNEL_SYSCALLS_DEFINE_H

#include "arch/riscv_isa.h"

namespace kernel {

struct ProcessTask;

void TrapRet(ProcessTask* process, riscv::Exception exception);
void ExcuteInitProcess(char* code);

namespace syscall {

int sys_fork();
int sys_write();
int sys_exec();

}  // namespace syscall
}  // namespace kernel

#endif