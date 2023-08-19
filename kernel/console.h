#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include "driver/basic_device.h"
#include "kernel/lock/spin_lock.h"
#include "kernel/resource.h"

namespace driver {
class BasicDevice;
}

namespace kernel {

class Console : Resource {
 public:
  Console(driver::BasicDevice* device);
  virtual void get_name(char* name) override;
  size_t read(char* p, size_t len) override;
  size_t write(char*p, size_t len) override;
  

 public:
  void InterruptHandler(const char*, size_t len);

  static const int buf_len = 256;
  struct Context {
    char buf[buf_len] = {0};
    uint8_t read_index = 0;
    uint8_t write_index = 0;
    SpinLock lock{};
  };
  Context context;
  driver::BasicDevice* device_ = nullptr;
};

}  // namespace kernel

#endif  // KERNEL_CONSOLE_H