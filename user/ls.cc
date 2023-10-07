#include "lib/lib.h"
#include "lib/string.h"
#include "lib/syscall.h"
#include "filesystem/inode_def.h"

void ShowFile(const char* name, int fd, fs::FileState* file_state = nullptr) {
  fs::FileState f;
  if (!file_state) {
    if (syscall::fstat(fd, &f) < 0) {
      printf("ls: cannot access '%s': trying to get file_state failed\n", name);
      return;
    }
  } else {
    f = *file_state;
  }
  if (f.type == fs::FileType::directory) {
    printf("   d%8d    \033[34m%s\n", f.size, name);
  } else if (f.type == fs::FileType::device) {
    printf("   c%8d    \033[33m%s\n", f.size, name);
  } else if (f.type == fs::FileType::regular_file) {
    printf("   -%8d    %s\n", f.size, name);
  }
  printf("\033[0m");
}

void DoList(const char* path) {
  int fd = syscall::open(path);
  if (fd < 0) {
    printf("ls: cannot access '%s': No such file or directory\n", path);
    return;
  }
  fs::FileState file_state;
  syscall::fstat(fd, &file_state);
  printf("---------------------\n");
  printf("type    size    name\n");
  if (file_state.type == fs::FileType::directory) {
    char buf[sizeof(fs::DirElement)];
    while (syscall::read(fd, buf, sizeof(fs::DirElement)) > 0) {
      auto* dir = reinterpret_cast<fs::DirElement*>(buf);
      char full_name[128];
      auto path_len = strlen(path);
      memcpy(full_name, path, path_len);
      if (full_name[path_len - 1] != '/') {
        full_name[path_len++] = '/';
      }
      memcpy(full_name + path_len, dir->name, strlen(dir->name) + 1);
      int child_fd = syscall::open(full_name);
      ShowFile(full_name, child_fd);
      syscall::close(child_fd);
    }
  } else {
    ShowFile(path, fd, &file_state);
  }
  syscall::close(fd);
}

int main(int argc, char** argv) {
  if (argc == 1) {
    char buf[256];
    if (syscall::getcwd(buf, 256)) {
      printf("ls: cannot access current path: path_name too long\n");
      return -1;
    }
    DoList(buf);
  } else {
    for (int i = 1; i < argc; i++) {
      DoList(argv[i]);
    }
  }
  return 0;
}