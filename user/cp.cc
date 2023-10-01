#include "filesystem/inode_def.h"
#include "lib/lib.h"
#include "lib/stl/smart_pointer.h"
#include "lib/stringpp.h"
#include "lib/syscall.h"
#include "lib/stl/vector.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    printf(PrintLevel::error, "cp: missing operand\nTry 'cp --help' for more information.\n");
    return -1;
  }
  bool is_multisource = argc > 3;
  char* dest = argv[argc - 1];
  int dest_fd = syscall::open(dest);
  fs::FileState dest_state{};
  if (!(dest_fd < 0)) {
    syscall::fstat(dest_fd, &dest_state);
  }
  if (is_multisource && (dest_fd < 0 || dest_state.type != fs::FileType::directory)) {
    printf(PrintLevel::error, "cp: target '%s' is not a directory\n", dest);
    return -1;
  }
  if (!is_multisource && (dest_fd > 0 && dest_state.type != fs::FileType::directory)) {
    printf(PrintLevel::error, "cp: dest target '%s' already exists\n", dest);
    return -1;
  }
  auto src_fds = lib::make_unique<lib::vector<int>>();
  for (int i = 1; i < (argc - 1); i++) {
    src_fds->push_back(syscall::open(argv[i]));
    if (src_fds->back() < 0) {
      printf(PrintLevel::error, "cp: cannot stat '%s': No such file or directory\n", argv[i]);
      return -1;
    }
  }
  lib::string dest_path = lib::string(dest);
  if (dest_state.type == fs::FileType::directory && dest_path[dest_path.size() - 1] != '/') {
    dest_path.append('/');
  }
  for (int i = 1; i < (argc - 1); i++) {
    lib::string target;
    if (dest_state.type == fs::FileType::directory) {
      target = dest_path + argv[i];
    } else {
      target = dest_path;
    }
    int target_fd = syscall::create(target.c_str(), fs::FileType::regular_file);
    char buf[512];
    int cnt;
    while ((cnt = syscall::read((*src_fds)[i - 1], buf, sizeof(buf))) > 0) {
      syscall::write(target_fd, buf, cnt);
    }
  }
  return 0;
}