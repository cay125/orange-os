# Initial process that execs /init.
# This code runs in user space.

#include "kernel/syscalls/syscall_num_def.h"

.globl __start__
__start__:
  la a0, init
  la a1, argv
  li a7, SYSCALL_exec
  ecall

exit:
  li a7, SYSCALL_exit
  ecall
  jal exit

init:
  .string "/init\0"

.p2align 2
argv:
  .long init
  .long 0
