#pragma once

#include "driver/virtio.h"
#include "kernel/sys_def/device_info.h"
#include <array>
#include <span>

namespace driver {
namespace virtio {
namespace net {

enum class feature_bit : uint64_t {
  VIRTIO_NET_F_CSUM = 1uL << 0,
  VIRTIO_NET_F_GUEST_CSUM = 1uL << 1,
  VIRTIO_NET_F_CTRL_GUEST_OFFLOADS = 1uL << 2,
  VIRTIO_NET_F_MTU = 1uL << 3,
  VIRTIO_NET_F_MAC = 1uL << 5,
  VIRTIO_NET_F_GUEST_TSO4 = 1uL << 7,
  VIRTIO_NET_F_GUEST_TSO6 = 1uL << 8,
  VIRTIO_NET_F_GUEST_ECN = 1uL << 9,
  VIRTIO_NET_F_GUEST_UFO = 1uL << 10,
  VIRTIO_NET_F_HOST_TSO4 = 1uL << 11,
  VIRTIO_NET_F_HOST_TSO6 = 1uL << 12, 
  VIRTIO_NET_F_HOST_ECN = 1uL << 13,
  VIRTIO_NET_F_HOST_UFO = 1uL << 14,
  VIRTIO_NET_F_MRG_RXBUF = 1uL << 15,
  VIRTIO_NET_F_STATUS = 1uL << 16,
  VIRTIO_NET_F_CTRL_VQ = 1uL << 17,
  VIRTIO_NET_F_CTRL_RX = 1uL << 18,
  VIRTIO_NET_F_CTRL_VLAN = 1uL << 19,
  VIRTIO_NET_F_GUEST_ANNOUNCE = 1uL << 21,
  VIRTIO_NET_F_MQ = 1uL << 22,
  VIRTIO_NET_F_CTRL_MAC_ADDR = 1uL << 23,
  VIRTIO_NET_F_RSC_EXT = 1uL << 61,
  VIRTIO_NET_F_STANDBY = 1uL << 62,
};

enum class status : uint16_t {
  VIRTIO_NET_S_LINK_UP = 1u << 0,
  VIRTIO_NET_S_ANNOUNCE = 1u << 2,
};

struct virtio_net_config {
  uint8_t mac[6];
  uint16_t status;
  uint16_t max_virtqueue_pairs;
  uint16_t mtu;
};

enum class GsoType : uint8_t {
  VIRTIO_NET_HDR_GSO_NONE = 0,
  VIRTIO_NET_HDR_GSO_TCPV4 = 1,
  VIRTIO_NET_HDR_GSO_UDP = 3,
  VIRTIO_NET_HDR_GSO_TCPV6 = 4,
  VIRTIO_NET_HDR_GSO_ECN = 0x80,
};

#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
#define VIRTIO_NET_HDR_F_DATA_VALID 2
#define VIRTIO_NET_HDR_F_RSC_INFO 4

struct virtio_net_hdr {
  uint8_t flags;
  GsoType gso_type;
  uint16_t hdr_len;
  uint16_t gso_size;
  uint16_t csum_start;
  uint16_t csum_offset;
  // uint16_t num_buffers;  // only available when the feature MRG_RXBUF is negotiated.
};

}

template <size_t N>
class RxBuffer {
 public:
  RxBuffer() : buf_{0}, packet_len_(0) {}
  void SetPacketLen(size_t packet_len) {
    packet_len_ = packet_len;
  }
  size_t PacketLen() const {
    return packet_len_;
  }
  const net::virtio_net_hdr* Header() {
    return reinterpret_cast<const net::virtio_net_hdr*>(buf_.data());
  }
  std::array<uint8_t, N>* Data() {
    return &buf_;
  }
  const std::array<uint8_t, N>* Data() const {
    return &buf_;
  }
  const uint8_t* Payload() const {
    return buf_.data() + sizeof(net::virtio_net_hdr);
  }

 private:
  std::array<uint8_t, N> buf_;
  size_t packet_len_;
};

class NetDevice : public DeviceViaMMIO {
  constexpr static size_t buf_len = 2048;
 public:
  NetDevice();
  bool Init(uint64_t virtio_addr) override;
  device_id GetDeviceId() override;
  void UsedBufferNotify() override;
  void ConfigChangeNotify() override;

  void Send(const std::span<uint8_t>& data);
  void Send(const uint8_t* data, size_t size);
  device_info::mac_info GetMacAddr();

 private:
  void DealRxBuffer();
  void RecycleRxBuffer();
  void DealTxBuffer();

  constexpr static uint8_t receive_queue_ind = 0;
  constexpr static uint8_t send_queue_ind = 1;
  std::array<RxBuffer<buf_len>, queue_buffer_size> rx_buffer_;
  std::array<std::array<uint32_t, 2>, queue_buffer_size> delayed_tx_desc;
  device_info::mac_info mac_info_;
  net::virtio_net_config* config_;
};

}
}