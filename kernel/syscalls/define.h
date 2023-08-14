#ifndef KERNEL_SYSCALLS_DEFINE_H
#define KERNEL_SYSCALLS_DEFINE_H

#include "arch/riscv_isa.h"

namespace lib {
class StreamBase;
}

namespace kernel {

struct ProcessTask;

void TrapRet(ProcessTask* process, riscv::Exception exception);
void ExcuteInitProcess(char* code, size_t size);
void ExecuteRet();
int ExecuteImpl(lib::StreamBase* stream, ProcessTask* process);

namespace syscall {

int sys_fork();
int sys_write();
int sys_read();
int sys_exec();
int sys_sleep();
int sys_open();
int sys_fstat();
int sys_mknod();
int sys_getpid();
int sys_exit();
int sys_wait();
int sys_getcwd();
int sys_chdir();
int sys_mkdir();
int sys_sbrk();

}  // namespace syscall
}  // namespace kernel

#endif