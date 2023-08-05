#ifndef LIB_SYSCALLS_H_
#define LIB_SYSCALLS_H_

#include "lib/types.h"
namespace syscall {
  
int write(int fd, const void* src, int size);
int fork();
int sleep(int seconds);
int open(const char*);
ssize_t read(int fd, char* buf, size_t count);
int fstat(int fd, void* f_stat);
int exec(const char* path);
int mknod(const char* path, uint8_t major, uint8_t minor);
int getpid();
int exit();
int wait();

}  // namespace syscall

#endif  // LIB_SYSCALLS_H_