#ifndef KERNEL_SYSCALLS_MANAGER_H
#define KERNEL_SYSCALLS_MANAGER_H

namespace kernel {
namespace syscall {

class Manager {
 public:
  static Manager* Instance();
  int DoWork(int num);
  int Sum();
 private:
  Manager() {}

  static Manager manager_;
  static int (*syscalls[])(void);
};

}  // namespace syscall
}  // namespace kernel

#endif  // KERNEL_SYSCALLS_MANAGER_H