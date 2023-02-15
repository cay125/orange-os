#include "kernel/lock/spin_lock.h"

namespace kernel {

void SpinLock::Lock() {
  while(flag_.test_and_set());
}

void SpinLock::UnLock() {
  flag_.clear();
}

}  // namespace kernel