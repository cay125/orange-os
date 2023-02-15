#ifndef KERNEL_LOCK_SPIN_LOCK_H
#define KERNEL_LOCK_SPIN_LOCK_H

#include <atomic>

namespace kernel {

class SpinLock {
 public:
  SpinLock() = default;
  SpinLock(const SpinLock&) = delete;
  SpinLock& operator= (const SpinLock&) = delete;

  void Lock();
  void UnLock();

 private:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

};

template <typename T>
class LockGuard {
 public:
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator= (const LockGuard&) = delete;
  LockGuard(T* lock) : lock_(lock) {
    lock_->Lock();
  }
  ~LockGuard() {
    lock_->UnLock();
  }

 private:
  T* lock_;
};

}  // namespace kernel

#endif