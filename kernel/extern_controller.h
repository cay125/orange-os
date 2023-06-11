#ifndef KERNEL_EXTERN_CONTROLLER_H
#define KERNEL_EXTERN_CONTROLLER_H

#include <array>
#include <utility>

#include "arch/riscv_plic.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace driver {
class BasicDevice;
}  // namespace driver

namespace kernel {

class ExternController : public lib::Singleton<ExternController> {
 public:
  friend class lib::Singleton<ExternController>;
  void Register(riscv::plic::irq e, driver::BasicDevice* device);
  void DoWork();

 private:
  driver::BasicDevice* GetDeviceByIRQ(uint32_t irq);

  // irq <--> device
  std::array<std::pair<riscv::plic::irq, driver::BasicDevice*>, 8> device_list_;
  uint32_t device_list_size_ = 0;
};

}  // namespace kernel

#endif  // KERNEL_EXTERN_CONTROLLER_H