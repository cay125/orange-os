#ifndef LIB_SYSCALLS_H_
#define LIB_SYSCALLS_H_

namespace syscall {
  
int write(int fd, const void* src, int size);
int fork();
int sleep(int seconds);

}  // namespace syscall

#endif  // LIB_SYSCALLS_H_