#include "driver/device_factory.h"

#include "kernel/config/memory_layout.h"
#include "kernel/utils.h"

namespace driver {

void DeviceFactory::InitDevices() {
  if (!blk_device0_.Init(memory_layout::VIRTIO0, riscv::plic::irq::VIRTIO0_IRQ)) {
    kernel::panic();
  }
}

driver::virtio::Device* DeviceFactory::GetDevice(DeviceList device) {
  if (device == DeviceList::disk0) {
    return &blk_device0_;    
  }
  return nullptr;
}

}  // namespace driver