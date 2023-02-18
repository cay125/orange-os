#ifndef CRITICAL_GUARD_H
#define CRITICAL_GUARD_H

#include "kernel/lock/spin_lock.h"

namespace kernel {

class CriticalGuard {
 public:
  CriticalGuard(SpinLock* lk = nullptr);
  ~CriticalGuard();

 private:
  bool is_intr_on_ = false;
  SpinLock* lk_ = nullptr;
};

}  // namespace kernel

#endif  // CRITICAL_GUARD_H