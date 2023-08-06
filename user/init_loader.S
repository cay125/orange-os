# Initial process that execs /init.
# This code runs in user space.

#include "kernel/syscalls/syscall_num_def.h"

.globl _start
_start:
  la a0, init
  li a7, SYSCALL_exec
  ecall

exit:
  li a7, SYSCALL_exit
  ecall
  jal exit

init:
  .string "/init\0"
