#include "driver/virtio_net.h"
#include "driver/negotiater_mmio.h"
#include "driver/transport_mmio.h"
#include "driver/virtio.h"
#include "kernel/printf.h"
#include <array>
#include <utility>

namespace driver {
namespace virtio {

NetDevice::NetDevice() : DeviceViaMMIO(0) {

}

bool NetDevice::Init(uint64_t virtio_addr) {
  addr_ = virtio_addr;
  if (!Validate()) {
    return false;
  }
  mmio_negotiater negotiater(this, {feature_bit::VIRTIO_F_ANY_LAYOUT}, {net::feature_bit::VIRTIO_NET_F_MAC, net::feature_bit::VIRTIO_NET_F_STATUS});
  negotiater.init(2);
  config_ = reinterpret_cast<net::virtio_net_config*>(addr_ + mmio_addr::Config);
  std::copy(config_->mac, config_->mac + mac_info_.size(), mac_info_.begin());
  kernel::printf("[virtio] device: %#x net_device mac_addr: %x %x %x %x %x %x\n", addr_, mac_info_[0], mac_info_[1], mac_info_[2], mac_info_[3], mac_info_[4], mac_info_[5]);

  for (uint32_t i = 0; i < queue_buffer_size; ++i) {
    mmio_transport transport(this, receive_queue_ind, rx_buffer_[i].Data());
    transport.trigger_notify();
  }

  return true;
}

device_info::mac_info NetDevice::GetMacAddr() {
  return mac_info_;
}

device_id NetDevice::GetDeviceId() {
  return device_id::network_card;
}

void NetDevice::DealRxBuffer() {
  kernel::CriticalGuard guard1(&lk_[receive_queue_ind]);
  __sync_synchronize();
  while (last_seen_used_idx_[receive_queue_ind] != queue[receive_queue_ind]->used.idx) {
    __sync_synchronize();
    auto& element = queue[receive_queue_ind]->used.ring[last_seen_used_idx_[receive_queue_ind] % queue_buffer_size];
    last_seen_used_idx_[receive_queue_ind] += 1;
    auto& buffer = rx_buffer_[element.id];
    buffer.SetPacketLen(element.len);
    if (interrupt_callback) {
      interrupt_callback(buffer.Payload(), buffer.PacketLen());
    }
    RecycleRxBuffer();
  }
}

void NetDevice::DealTxBuffer() {
  kernel::CriticalGuard guard(&lk_[send_queue_ind]);
  __sync_synchronize();
  while (last_seen_used_idx_[send_queue_ind] != queue[send_queue_ind]->used.idx) {
    __sync_synchronize();
    auto& element = queue[send_queue_ind]->used.ring[last_seen_used_idx_[send_queue_ind] % queue_buffer_size];
    internal_data_[element.id].waiting_flag = false;
    for (auto desc : delayed_tx_desc[element.id]) {
      FreeDesc(desc, send_queue_ind);
    }
    last_seen_used_idx_[send_queue_ind] += 1;
  }
}

void NetDevice::RecycleRxBuffer() {
  queue[receive_queue_ind]->avail.idx += 1;
  __sync_synchronize();
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNotify, receive_queue_ind);
}

void NetDevice::Send(const std::span<uint8_t>& data) {
  Send(data.data(), data.size());
}

void NetDevice::Send(const uint8_t* data, size_t size) {
  const net::virtio_net_hdr hdr{};
  if (size == 0) {
    kernel::printf("NetDevice: got empty packet when send\n");
    return;
  } else {
    const auto data_pair = std::make_pair(data, size);
    mmio_transport transport(this, send_queue_ind, &hdr, &data_pair);
    transport.trigger_notify();
    auto descs = transport.get_descs();
    delayed_tx_desc[descs[0]] = descs;
  }
}

void NetDevice::UsedBufferNotify() {
  DealRxBuffer();
  DealTxBuffer();
}

void NetDevice::ConfigChangeNotify() {}

}
}