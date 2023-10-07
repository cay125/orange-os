#include "driver/negotiater_mmio.h"
#include "driver/virtio_blk.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include "kernel/virtual_memory.h"

namespace driver {
namespace virtio {

BlockDevice::BlockDevice() : DeviceViaMMIO(0)  {

}

bool BlockDevice::Init(uint64_t virtio_addr) {
  addr_  = virtio_addr;
  if (!Validate()) {
    return false;
  }
  mmio_negotiater negotiater(
    this,
    {feature_bit::VIRTIO_F_RING_INDIRECT_DESC, feature_bit::VIRTIO_F_NOTIFY_ON_EMPTY},
    {blk::feature_bit::VIRTIO_BLK_F_SEG_MAX, blk::feature_bit::VIRTIO_BLK_F_GEOMETRY, blk::feature_bit::VIRTIO_BLK_F_BLK_SIZE, blk::feature_bit::VIRTIO_BLK_F_FLUSH, blk::feature_bit::VIRTIO_BLK_F_TOPOLOGY, blk::feature_bit::VIRTIO_BLK_F_DISCARD, blk::feature_bit::VIRTIO_BLK_F_WRITE_ZEROES}
  );
  bool ret = negotiater.init(1);
  if (!ret) {
    return false;
  }

  auto* config = reinterpret_cast<blk::virtio_blk_config*>(addr_ + mmio_addr::Config);
  capacity_ = config->capacity * blk::unit;
  kernel::printf("[virtio] device: %#x capacity: %d byte\n", addr_, capacity_);

  return true;
}

void BlockDevice::UsedBufferNotify() {
  kernel::CriticalGuard guard(&lk_[0]);
  __sync_synchronize();
  while (last_seen_used_idx_[0] != queue[0]->used.idx) {
    __sync_synchronize();
    auto& element = queue[0]->used.ring[last_seen_used_idx_[0] % queue_buffer_size];
    internal_data_[element.id].waiting_flag = false;
    last_seen_used_idx_[0] += 1;
    kernel::Schedueler::Instance()->Wakeup(&channel_);
  }
}

void BlockDevice::ConfigChangeNotify() {}

uint64_t BlockDevice::Capacity() {
  return capacity_;
}

device_id BlockDevice::GetDeviceId() {
  return device_id::block_device;
}

}
}
