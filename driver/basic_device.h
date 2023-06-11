#ifndef DRIVER_BASIC_DEVICE_H_
#define DRIVER_BASIC_DEVICE_H_

#include "arch/riscv_plic.h"

namespace driver {

class BasicDevice {
 public:
  virtual bool Init(uint64_t addr) = 0;
  virtual void ProcessInterrupt() = 0;
};

}  // namespace driver

#endif  // DRIVER_BASIC_DEVICE_H_
