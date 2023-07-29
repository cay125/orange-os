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

 public:
  void(*interrupt_callback)(const char*, size_t len) = nullptr;
};

}  // namespace driver

#endif  // DRIVER_BASIC_DEVICE_H_
