#include "kernel/syscalls/syscall_num_def.h"

.global _ZN7syscall6dlopenEPKc
_ZN7syscall6dlopenEPKc:
  li  a7, SYSCALL_dlopen
  ecall
  addi sp, sp, -8
  sd ra, 0(sp)
  call _Z22sys_resolve_init_arrayiPPFvvES1_
  ld ra, 0(sp)
  addi sp, sp, 8
  ret

.global _ZN7syscall7dlcloseEv
_ZN7syscall7dlcloseEv:
  li  a7, SYSCALL_dlclose
  ecall
  ret
