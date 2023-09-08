#include "driver/virtio_blk.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include "kernel/virtual_memory.h"

namespace driver {
namespace virtio {

BlockDevice::BlockDevice() : Device()  {

}

bool BlockDevice::Init(uint64_t virtio_addr) {
  addr_  = virtio_addr;
  if (!Validate()) {
    return false;
  }
  // reset device
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, 0);
  uint32_t status = 0;
  // OS discover device
  status |= status_field::ACKNOWLEDGE;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

  // OS know how to drive device
  status |= status_field::DRIVER;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

  // read and set feature bits
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DeviceFeaturesSel, 0);
  uint32_t feature_bits_low = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::DeviceFeatures);
  kernel::printf("[virtio] device: %#x feature availible  0-31: %#x\n", addr_, feature_bits_low);
  feature_bits_low &= ~blk::feature_bit::VIRTIO_BLK_F_RO;
  feature_bits_low &= ~blk::feature_bit::VIRTIO_BLK_F_SCSI;
  feature_bits_low &= ~blk::feature_bit::VIRTIO_BLK_F_CONFIG_WCE;
  feature_bits_low &= ~blk::feature_bit::VIRTIO_BLK_F_MQ;
  feature_bits_low &= ~feature_bit::VIRTIO_F_ANY_LAYOUT;
  feature_bits_low &= ~feature_bit::VIRTIO_F_RING_INDIRECT_DESC;
  feature_bits_low &= ~feature_bit::VIRTIO_F_RING_EVENT_IDX;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DriverFeaturesSel, 0);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DriverFeatures, feature_bits_low);

  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DeviceFeaturesSel, 1);
  uint32_t feature_bits_high = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::DeviceFeatures);
  kernel::printf("[virtio] device: %#x feature availible 32-63: %#x\n", addr_, feature_bits_high);

  // OS finish feature bits setting
  status |= status_field::FEATURES_OK;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

  // re-read device status to ensure the FEATURES_OK bit is still set
  auto device_status = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::Status);
  kernel::printf("[virtio] device: %#x status: %#x\n", addr_, device_status);
  if (!(device_status & status_field::FEATURES_OK)) {
    kernel::printf("[virt] device: %#x feature_bit is not set, config failed\n", addr_);
    return false;
  }

  // set page_size
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::GuestPageSize, memory_layout::PGSIZE);

  auto* config = reinterpret_cast<blk::virtio_blk_config*>(addr_ + mmio_addr::Config);
  capacity_ = config->capacity * blk::unit;
  kernel::printf("[virtio] device: %#x capacity: %d byte\n", addr_, capacity_);

  // set virt-queue
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueSel, 0);
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueuePFN) != 0) {
    kernel::printf("[virtio] device: %#x queue_fpn is not zero, setting failed\n", addr_);
    return false;
  }
  auto max_queue_buffer = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueueNumMax);
  kernel::printf("[virtio] device: %#x max queue buffer: %d\n", addr_, max_queue_buffer);
  if (max_queue_buffer < queue_buffer_size) {
    return false;
  }
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNum, queue_buffer_size);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueAlign, memory_layout::PGSIZE);

  auto* page = kernel::VirtualMemory::Instance()->AllocContinuousPage(2);
  if (!page) {
    kernel::printf("[virtio] device: %#x alloc memory page failed\n", addr_);
    return false;
  }
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueuePFN, reinterpret_cast<uint64_t>(page) / memory_layout::PGSIZE);
  queue[0] = reinterpret_cast<virt_queue*>(page);

  // OS finish device setting
  status |= status_field::DRIVER_OK;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

  return true;
}

bool BlockDevice::Validate() {
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::MagicValue) != magic_num) return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::Version) != device_version::Legacy) return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::DeviceID) != device_id::block_device) return false;
  kernel::printf("[virtio] device: %#x vendor id: %#x\n", addr_, MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::VendorID));
  return true;
}

void BlockDevice::ProcessInterrupt() {
  kernel::CriticalGuard guard(&lk_[0]);
  auto interrupt_status = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::InterruptStatus);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::InterruptACK, interrupt_status & 0x3);
  __sync_synchronize();
  while (last_seen_used_idx_[0] != queue[0]->used.idx) {
    __sync_synchronize();
    auto& element = queue[0]->used.ring[last_seen_used_idx_[0] % queue_buffer_size];
    internal_data_[element.id].waiting_flag = false;
    last_seen_used_idx_[0] += 1;
    kernel::Schedueler::Instance()->Wakeup(&channel_);
  }
}

uint64_t BlockDevice::Capacity() {
  return capacity_;
}

device_id BlockDevice::GetDeviceId() {
  return device_id::block_device;
}

}
}
