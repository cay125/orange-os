#include "lib/common.h"
#include "lib/lib.h"
#include "lib/syscall.h"
#include "filesystem/inode_def.h"
#include "kernel/resource.h"

int main(void) {
  const char* console_path = "/dev/console";
  int console = syscall::open(console_path);
  if (console < 0) {
    printf(PrintLevel::info, "sh: trying to mount %s\n", console_path);
    int mknod_ret = syscall::mknod(console_path, kernel::resource_id::console_major, kernel::resource_id::console_minor);
    if (mknod_ret < 0) {
      printf(PrintLevel::error, "sh: mount %s failed, program will exit right now\n", console_path);
      syscall::exit();
    }
    printf(PrintLevel::info, "sh: mount %s succ\n", console_path);
    console = syscall::open("/dev/console");
  }
  fs::FileState state{};
  syscall::fstat(console, &state);
  printf(PrintLevel::info, "sh: console inode_index: %d, size: %d, type: %s\n", state.inode_index, state.size, fs::FileTypeName(state.type));

  while(true) {
    char buf[128];
    syscall::getcwd(buf, 128);
    printf("\033[34;1m%s\033[32;1m$\033[0m ", buf);
    int n = syscall::read(console, buf, 128);
    buf[n - 1] = '\0';
    char split_ret[16][32];
    lib::common::SplitString(buf, split_ret, ' ');
    int sub_process = syscall::fork();
    if (sub_process < 0) {
      printf("trying to create a new process failed\n");
      continue;
    }
    if (sub_process == 0) {
      syscall::exec(split_ret[0]);
      printf("%s: command not found\n", split_ret[0]);
      syscall::exit();
    }
    syscall::wait();
  }
  // never reach
  syscall::exit();
}