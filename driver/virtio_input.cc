#include "driver/negotiater_mmio.h"
#include "driver/transport_mmio.h"
#include "driver/virtio_input.h"
#include "kernel/printf.h"
#include "kernel/sys_def/input_event_code.h"
#include "lib/common.h"

namespace driver {
namespace virtio {

InputDevice::InputDevice() : DeviceViaMMIO(0) {}

bool InputDevice::Init(uint64_t virtio_addr) {
  addr_ = virtio_addr;
  if (!Validate()) {
    return false;
  }
  mmio_negotiater negotiater(
    this,
    {feature_bit::VIRTIO_F_NOTIFY_ON_EMPTY, feature_bit::VIRTIO_F_ANY_LAYOUT, feature_bit::VIRTIO_F_RING_INDIRECT_DESC}
  );
  negotiater.init(2);

  config_ = reinterpret_cast<input::virtio_input_config*>(addr_ + mmio_addr::Config);
  config_->select = lib::common::literal(input::virtio_input_config_select::VIRTIO_INPUT_CFG_ID_NAME);
  config_->subsel = 0;
  kernel::printf("[virtio] device: %#x input_device name: %s\n", addr_, config_->u.string);

  for (uint32_t i = 0; i < queue_buffer_size; ++i) {
    mmio_transport transport(this, 0, addr_, &input_event_[i]);
    transport.trigger_notify();
  }

  return true;
}

device_id InputDevice::GetDeviceId() {
  return device_id::input_device;
}

void InputDevice::UsedBufferNotify() {
  kernel::CriticalGuard guard1(&lk_[0]);
  __sync_synchronize();
  while (last_seen_used_idx_[0] != queue[0]->used.idx) {
    __sync_synchronize();
    auto& element = queue[0]->used.ring[last_seen_used_idx_[0] % queue_buffer_size];
    last_seen_used_idx_[0] += 1;
    auto& event = input_event_[element.id];
    if (event.type == event_def::EV_REL && event.code == event_def::REL_X) {
      position_.x += event.value;
    } else if (event.type == event_def::EV_REL && event.code == event_def::REL_Y) {
      position_.y += event.value;
    }
    queue[0]->avail.idx += 1;
    __sync_synchronize();
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNotify, 0);
  }
}

void InputDevice::ConfigChangeNotify() {}

input::Position InputDevice::GetPosition() const {
  return position_;
}

}
}