#include "kernel/syscalls/manager.h"

#include "kernel/printf.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/syscall_num_def.h"

namespace kernel {
namespace syscall {

int mock_syscall() {
  printf("Non-implement syscall\n");
  return 0;
}

int (*Manager::syscalls[])(void) = {
  [SYSCALL_fork]    = sys_fork,
  [SYSCALL_write]   = sys_write,
  [SYSCALL_read]    = sys_read,
  [SYSCALL_exec]    = sys_exec,
  [SYSCALL_close]   = mock_syscall,
  [SYSCALL_exit]    = sys_exit,
  [SYSCALL_sleep]   = sys_sleep,
  [SYSCALL_open]    = sys_open,
  [SYSCALL_fstat]   = sys_fstat,
  [SYSCALL_mknod]   = sys_mknod,
  [SYSCALL_getpid]  = sys_getpid,
  [SYSCALL_wait]    = sys_wait,
};

int Manager::Sum() {
  return sizeof(syscalls) / sizeof(syscalls[0]);
}

int Manager::DoWork(int num) {
  if (num >= Sum() || num < 0 || !syscalls[num]) {
    printf("Invalid syscall num: %d, max_num: %d", num, Sum() - 1);
    return -1;
  }
  return syscalls[num]();
}

}  // namespace syscall
}  // namespace kernel