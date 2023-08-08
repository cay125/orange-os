#include "lib/lib.h"
#include "lib/syscall.h"

int main(int argc, char** argv) {
  char buf[128];
  if (syscall::getcwd(buf, 128) < 0) {
    printf("pwd: failed\n");
    return -1;
  }
  printf("%s\n", buf);
  return 0;
}
