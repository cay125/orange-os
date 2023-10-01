#include "driver/virtio.h"

#include "kernel/printf.h"
#include "kernel/utils.h"
#include "lib/string.h"

namespace driver {
namespace virtio {

int Device::AllocDesc(int queue_index) {
  for (uint32_t i = 0; i < queue_buffer_size; ++i) {
    if ((bit_map_[queue_index][i / 8] & (1u << (i % 8))) == 0) {
      bit_map_[queue_index][i / 8] |= 1u << (i % 8);
      return i;      
    }
  }
  return -1;
}

Device::Device() : channel_(this) {

}

void Device::FreeDesc(uint32_t desc_index, int queue_index) {
  if (desc_index >= queue_buffer_size) {
    kernel::panic("Invalid desc_index[%d] in virt-io: too bigger", desc_index);
  }
  if ((bit_map_[queue_index][desc_index / 8] & (1 << desc_index % 8)) == 0) {
    kernel::panic("Invalid desc_index[%d] in virt-io: weird num", desc_index);
  }
  bit_map_[queue_index][desc_index / 8] &= ~(1 << (desc_index % 8));
  memset(queue[queue_index]->desc + desc_index, 0, sizeof(virtq_desc));
}

void Device::FreeDesc(std::initializer_list<uint32_t> desc_list, int queue_index) {
  for (auto desc : desc_list) {
    FreeDesc(desc, queue_index);
  }
}

bool DeviceViaMMIO::Validate() {
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::MagicValue) != magic_num)
    return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::Version) != device_version::Legacy)
    return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::DeviceID) != GetDeviceId())
    return false;
  kernel::printf("[virtio] device: %#x vendor id: %#x\n", addr_, MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::VendorID));
  return true;
}

}  // namespace virtio
}  // namespace driver