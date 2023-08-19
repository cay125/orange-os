#ifndef DRIVER_BASIC_DEVICE_H_
#define DRIVER_BASIC_DEVICE_H_

#include "lib/types.h"

namespace driver {

class BasicDevice {
 public:
  virtual bool Init(uint64_t addr) = 0;
  virtual void ProcessInterrupt() = 0;
  virtual void RegisterInterruptCallback(void(*fun)(const char*, size_t len)) {
    interrupt_callback = fun;
  }
  bool IsWritable() {
    return is_writable_;
  }
  virtual void Write(const char* buf, size_t size) {}

 protected:
  bool is_writable_ = false;
  void(*interrupt_callback)(const char*, size_t len) = nullptr;
};

}  // namespace driver

#endif  // DRIVER_BASIC_DEVICE_H_
