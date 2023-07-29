#ifndef KERNEL_GLOBAL_CHANNEL
#define KERNEL_GLOBAL_CHANNEL

#include "kernel/process.h"

namespace kernel {

class GlobalChannel {
 public:
  static Channel* sleep_channel() {
    return &sleep_channel_;
  }

  static Channel* console_channel() {
    return &console_channel_;
  }

 private:
  static Channel sleep_channel_;
  static Channel console_channel_;
};

}  // namespace kernel

#endif  // KERNEL_GLOBAL_CHANNEL
