#ifndef KERNEL_SYSCALLS_DEFINE_H
#define KERNEL_SYSCALLS_DEFINE_H

#include "arch/riscv_isa.h"

namespace kernel {

struct ProcessTask;

void TrapRet(ProcessTask* process, riscv::Exception exception);
void ExcuteInitProcess(char* code, size_t size);
void ExecuteRet();

namespace syscall {

int sys_fork();
int sys_write();
int sys_read();
int sys_exec();
int sys_sleep();
int sys_open();
int sys_fstat();

}  // namespace syscall
}  // namespace kernel

#endif