#ifndef KERNEL_EXTERN_CONTROLLER_H
#define KERNEL_EXTERN_CONTROLLER_H

#include <array>
#include <utility>

#include "arch/riscv_plic.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace driver {
namespace virtio {
class Device;
}  // namespace virtio
}  // namespace driver

namespace kernel {

class ExternController : public lib::Singleton<ExternController> {
 public:
  friend class lib::Singleton<ExternController>;
  void Register(riscv::plic::irq e, driver::virtio::Device* device);
  void DoWork();

 private:
  driver::virtio::Device* GetDeviceByIRQ(uint32_t irq);

  // irq <--> device
  std::array<std::pair<riscv::plic::irq, driver::virtio::Device*>, 8> device_list_;
  uint32_t device_list_size_ = 0;
};

}  // namespace kernel

#endif  // KERNEL_EXTERN_CONTROLLER_H