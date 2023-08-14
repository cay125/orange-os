#include "lib/lib.h"
#include "lib/syscall.h"

int main(int argc, char** argv) {
  if (argc == 1) {
    printf(PrintLevel::error, "mkdir: missing operand\nTry 'mkdir --help' for more information.\n");    
    return -1;
  }
  for (int i = 1; i < argc; ++i) {
    if (syscall::mkdir(argv[i]) < 0) {
      printf(PrintLevel::error, "mkdir: create %s failed\n", argv[i]);
      continue;
    }
  }
  return 0;
}