#include "lib/syscall.h"
#include "lib/lib.h"

int main(void) {
  while (true) {
    printf("\033[35;1minit: starting sh\033[0m\n");
    int sh_pid = syscall::fork();
    if (sh_pid == 0) {
      syscall::exec("sh");
      printf("trying to exec sh faild\n");
      syscall::exit();
    }
    while (true) {
      int exit_pid = syscall::wait();
      if (exit_pid == sh_pid) {
        break;
      } else {
        // parentless process exit, do nothing
      }
    }
  }
  return 0;
}