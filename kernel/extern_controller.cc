#include "kernel/extern_controller.h"

#include "driver/basic_device.h"
#include "kernel/config/memory_layout.h"
#include "kernel/utils.h"
#include "lib/common.h"

namespace kernel {

driver::BasicDevice* ExternController::GetDeviceByIRQ(uint32_t irq) {
  for (uint32_t i = 0; i < device_list_size_; ++i) {
    if (lib::common::literal(device_list_[i].first) == irq) {
      return device_list_[i].second;
    }
  }
  return nullptr;
}

void ExternController::DoWork() {
  uint32_t irq = MEMORY_MAPPED_IO_R_WORD(memory_layout::PLIC_CLAIM(cpu_id()));
  driver::BasicDevice* device = GetDeviceByIRQ(irq);
  if (!device) {
    panic("No device satisfied irq: %d", irq);
  }
  device->ProcessInterrupt();
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_CLAIM(cpu_id()), irq);
}

void ExternController::Register(riscv::plic::irq e, driver::BasicDevice* device) {
  device_list_[device_list_size_].first = e;
  device_list_[device_list_size_].second = device;
  device_list_size_ += 1;
}

}  // namespace kernel