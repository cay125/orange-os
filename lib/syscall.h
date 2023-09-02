#ifndef LIB_SYSCALLS_H_
#define LIB_SYSCALLS_H_

#include "kernel/sys_def/device_info.h"
#include "lib/types.h"
#include "filesystem/inode_def.h"
namespace syscall {
  
int write(int fd, const void* src, int size);
int fork();
int sleep(int seconds);
int open(const char*);
ssize_t read(int fd, char* buf, size_t count);
int fstat(int fd, void* f_stat);
int exec(const char* path, const char** argv);
int exec(const char* path, char** argv);
int mknod(const char* path, uint8_t major, uint8_t minor);
int getpid();
int exit(int);
int wait(int* exit = nullptr);
int getcwd(char* buf, size_t max_len);
int chdir(const char*);
int mkdir(const char*);
char* sbrk(uint32_t);
int uptime();
int create(const char* path, fs::FileType type);
int get_screen_info(device_info::screen_info* screen_info);
uint8_t* framebuffer(size_t size);
int frame_flush();

}  // namespace syscall

#endif  // LIB_SYSCALLS_H_