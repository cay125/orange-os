#pragma once

#include "driver/transport_mmio.h"
#include "driver/virtio.h"
#include "kernel/printf.h"
#include "kernel/utils.h"
#include "lib/types.h"
#include <array>
#include <type_traits>

namespace driver {
namespace virtio {
namespace blk {

constexpr uint32_t unit = 512;

enum class feature_bit : uint64_t {
  VIRTIO_BLK_F_BARRIER = 1uL << 0,
  VIRTIO_BLK_F_SIZE_MAX = 1ul << 1,
  VIRTIO_BLK_F_SEG_MAX = 1uL << 2,
  VIRTIO_BLK_F_GEOMETRY = 1uL << 4,
  VIRTIO_BLK_F_RO = 1uL << 5,
  VIRTIO_BLK_F_BLK_SIZE = 1uL << 6,
  VIRTIO_BLK_F_SCSI = 1uL << 7,
  VIRTIO_BLK_F_FLUSH = 1uL << 9,
  VIRTIO_BLK_F_TOPOLOGY = 1uL << 10,
  VIRTIO_BLK_F_CONFIG_WCE = 1uL << 11,
  VIRTIO_BLK_F_MQ = 1uL << 12,
  VIRTIO_BLK_F_DISCARD = 1uL << 13,
  VIRTIO_BLK_F_WRITE_ZEROES = 1uL << 14,
};

enum class req_type : uint32_t {
  VIRTIO_BLK_T_IN = 0,
  VIRTIO_BLK_T_OUT = 1,
  VIRTIO_BLK_T_FLUSH = 4,
  VIRTIO_BLK_T_DISCARD = 11,
  VIRTIO_BLK_T_WRITE_ZEROES = 13,
};

struct virtio_blk_req {
  req_type type;
  uint32_t reserved;
  uint64_t sector;
};

enum class status_type : uint8_t {
  VIRTIO_BLK_S_OK = 0,
  VIRTIO_BLK_S_IOERR = 1,
  VIRTIO_BLK_S_UNSUPP = 2,
};

struct virtio_blk_resp {
  status_type status;
};

struct virtio_blk_config {
  uint64_t capacity;
  uint32_t size_max;
  uint32_t seg_max;
  struct virtio_blk_geometry {
    uint16_t cylinders;
    uint8_t heads;
    uint8_t sectors;
  } geometry;
  uint32_t blk_size;
  struct virtio_blk_topology {
    // # of logical blocks per physical block (log2)
    uint8_t physical_block_exp;
    // offset of first aligned logical block
    uint8_t alignment_offset;
    // suggested minimum I/O size in blocks
    uint16_t min_io_size;
    // optimal (suggested maximum) I/O size in blocks
    uint32_t opt_io_size;
  } topology;
  uint8_t writeback;
  uint8_t unused0[3];
  uint32_t max_discard_sectors;
  uint32_t max_discard_seg;
  uint32_t discard_sector_alignment;
  uint32_t max_write_zeroes_sectors;
  uint32_t max_write_zeroes_seg;
  uint8_t write_zeroes_may_unmap;
  uint8_t unused1[3];
};

}  // namespace blk

template <typename T>
struct MetaData {
  T* buf = nullptr;
  uint64_t block_index;
};

class BlockDevice : public Device {
 public:
  BlockDevice();
  bool Init(uint64_t virtio_addr) override;
  device_id GetDeviceId() override;
  void ProcessInterrupt() override;

  template <Operation op, typename T>
  auto* wrap_blk_data(T* blk_data) {
    if constexpr (std::is_const_v<T> || op == Operation::write) {
      return reinterpret_cast<const std::array<uint8_t, block_size>*>(blk_data);
    } else {
      return reinterpret_cast<std::array<uint8_t, block_size>*>(blk_data);
    }
  }

  template<Operation op, typename T, std::enable_if_t<!(op == Operation::read && std::is_const_v<T>), bool> = true>
  bool Operate(MetaData<T>* meta_data) {
    if (!meta_data->buf) {
      kernel::panic("BlockDevice::Operate: Got empty data buf\n");
    }
    const blk::virtio_blk_req req{op == Operation::read ? blk::req_type::VIRTIO_BLK_T_IN : blk::req_type::VIRTIO_BLK_T_OUT, 0, meta_data->block_index};
    blk::virtio_blk_resp resp;
    mmio_transport transport(this, 0, addr_, &req, wrap_blk_data<op>(meta_data->buf), &resp);
    transport.trigger_notify();
    transport.wait_complete();
    if (resp.status != blk::status_type::VIRTIO_BLK_S_OK) {
      const char* status_msg = resp.status == blk::status_type::VIRTIO_BLK_S_IOERR ? "io error" : "unsupported operation";
      kernel::printf("BlkDevice Operate failed: %s", status_msg);
      return false;
    }
    return true;
  }

  uint64_t Capacity();
  auto GetSupportedOperation() {
    return std::array<Operation, 2>{Operation::read, Operation::write};
  }

 private:
  bool Validate();

  uint64_t addr_ = 0;
  uint64_t capacity_ = 0;
};

}
}