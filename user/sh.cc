#include "lib/common.h"
#include "lib/lib.h"
#include "lib/string.h"
#include "lib/syscall.h"
#include "filesystem/inode_def.h"
#include "kernel/resource.h"
#include "kernel/sys_def/syscall_err.h"

int main(void) {
  const char* console_path = "/dev/console";
  int console = syscall::open(console_path);
  if (console < 0) {
    printf(PrintLevel::info, "sh: trying to mount %s\n", console_path);
    int mknod_ret = syscall::mknod(console_path, kernel::resource_id::console_major, kernel::resource_id::console_minor);
    if (mknod_ret < 0) {
      printf(PrintLevel::error, "sh: mount %s failed, program will exit right now\n", console_path);
      return -1;
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
    printf("\033[34;1m%s\033[32;1m$\033[0m > ", buf);
    int n = syscall::read(console, buf, 128);
    if (buf[n - 1] == '\n') {
      buf[n - 1] = '\0';
    }
    char split_ret[16][32];
    int token_num = lib::common::SplitString(buf, split_ret, ' ');
    if (token_num < 0) {
      printf(PrintLevel::error, "sh: command argument to mush\n");
      continue;
    }
    char* argv[16 + 1];
    for (int i = 0; i < token_num; ++i) {
      argv[i] = split_ret[i];
    }
    argv[token_num] = nullptr;
    if (strcmp(argv[0], "cd") == 0) {
      if (token_num > 2) {
        printf("cd: too many arguments\n");
        continue;
      }
      int cd_ret;
      if (token_num == 1) {
        cd_ret = syscall::chdir("/");
      } else {
        cd_ret = syscall::chdir(argv[1]);
      }
      if (cd_ret == return_code::no_such_file_or_directory) {
        printf("sh: cd: %s: No such file or directory\n", argv[1]);
      } else if (cd_ret == return_code::not_a_directory) {
        printf("sh: cd: %s: Not a directory\n", argv[1]);
      }
      continue;
    }
    int sub_process = syscall::fork();
    if (sub_process < 0) {
      printf("trying to create a new process failed\n");
      continue;
    }
    if (sub_process == 0) {
      syscall::exec(split_ret[0], argv);
      printf("%s: command not found\n", split_ret[0]);
      return -1;
    }
    syscall::wait();
  }
  // never reach
  return 0;
}