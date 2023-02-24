#ifndef KERNEL_SYSCALLS_MANAGER_H
#define KERNEL_SYSCALLS_MANAGER_H

#include "lib/singleton.h"

namespace kernel {
namespace syscall {

class Manager : public lib::Singleton<Manager> {
 public:
  friend class lib::Singleton<Manager>;
  int DoWork(int num);
  int Sum();
 private:
  Manager() {}

  static int (*syscalls[])(void);
};

}  // namespace syscall
}  // namespace kernel

#endif  // KERNEL_SYSCALLS_MANAGER_H