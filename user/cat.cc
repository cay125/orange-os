#include "filesystem/inode_def.h"
#include "kernel/sys_def/descriptor_def.h"
#include "lib/lib.h"
#include "lib/syscall.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    return 0;
  }
  for (int i = 1; i < argc; i++) {
    int fd = syscall::open(argv[i]);
    if (fd < 0) {
      printf("cat: %s: No such file or directory\n", argv[i]);
      continue;
    }
    fs::FileState file_state;
    syscall::fstat(fd, &file_state);
    if (file_state.type == fs::FileType::directory || file_state.type == fs::FileType::device) {
      printf("cat: %s: Not a regular file\n", argv[i]);
      continue;
    }
    int n;
    char buf[512];
    while ((n = syscall::read(fd, buf, sizeof(buf))) > 0) {
      syscall::write(descriptor_def::io::console, buf, n);
    }
    syscall::write(descriptor_def::io::console, reinterpret_cast<const char*>(0x0), 5);
    printf("\n");
  }
  return 0;
}
