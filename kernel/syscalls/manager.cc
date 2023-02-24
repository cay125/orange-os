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
  [SYSCALL_read]    = mock_syscall,
  [SYSCALL_exec]    = mock_syscall,
  [SYSCALL_close]   = mock_syscall,
  [SYSCALL_exit]    = mock_syscall,
  [SYSCALL_sleep]   = sys_sleep,
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