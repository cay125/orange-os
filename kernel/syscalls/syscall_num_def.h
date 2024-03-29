#ifndef KERNEL_SYSCALLS_SYSCALL_NUM_DEF
#define KERNEL_SYSCALLS_SYSCALL_NUM_DEF

#define SYSCALL_fork        0
#define SYSCALL_write       1
#define SYSCALL_read        2
#define SYSCALL_exec        3
#define SYSCALL_close       4
#define SYSCALL_exit        5
#define SYSCALL_sleep       6
#define SYSCALL_open        7
#define SYSCALL_fstat       8
#define SYSCALL_mknod       9
#define SYSCALL_getpid     10
#define SYSCALL_wait       11
#define SYSCALL_getcwd     12
#define SYSCALL_chdir      13
#define SYSCALL_mkdir      14
#define SYSCALL_sbrk       15
#define SYSCALL_uptime     16
#define SYSCALL_create     17
#define SYSCALL_get_screen_info 18
#define SYSCALL_framebuffer 19
#define SYSCALL_frame_flush 20
#define SYSCALL_setup_cursor 21
#define SYSCALL_move_cursor 22
#define SYSCALL_detach_framebuffer 23
#define SYSCALL_dlopen 24
#define SYSCALL_dlclose 25

#endif
