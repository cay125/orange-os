#ifndef DRIVER_VIRTIO_H
#define DRIVER_VIRTIO_H

#include <array>
#include <initializer_list>

#include "arch/riscv_plic.h"
#include "kernel/config/memory_layout.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace driver {
namespace virtio {

constexpr uint32_t magic_num = 0x74726976;
constexpr uint32_t queue_buffer_size = 32;
constexpr uint32_t block_size = 512;

enum class mmio_addr : uint64_t {
  MagicValue = 0x000,
  Version = 0x004,
  DeviceID = 0x008,
  VendorID = 0x00c,
  DeviceFeatures = 0x010,
  DeviceFeaturesSel = 0x014,
  DriverFeatures = 0x020,
  DriverFeaturesSel = 0x024,
  GuestPageSize = 0x028,
  QueueSel = 0x030,
  QueueNumMax = 0x034,
  QueueNum = 0x038,
  QueueAlign = 0x03c,
  QueuePFN = 0x040,
  QueueReady = 0x044,
  QueueNotify = 0x050,
  InterruptStatus = 0x060,
  InterruptACK = 0x064,
  Status = 0x070,
  QueueDescLow = 0x080,
  QueueDescHigh = 0x084,
  QueueDriverLow = 0x090,
  QueueDriverHigh = 0x094,
  QueueDeviceLow = 0x0a0,
  QueueDeviceHigh = 0x0a4,
  ConfigGeneration = 0x0fc,
  Config = 0x100,
};

enum class device_version : uint8_t {
  None = 0,
  Legacy = 1,  // before v1.1
  Latest = 2,
};

enum class device_id : uint8_t {
  invalid = 0,
  network_card = 1,
  block_device = 2,
  console = 3,
  entropy_source = 4,
  memory_ballooning = 5,
  ioMemory = 6,
  rpmsg = 7,
  SCSI_host = 8,
  _9P_transport = 9,
  mac80211_wlan = 10,
  rproc_serial = 11,
  virtio_CAIF = 12,
  memory_balloon = 13,
  GPU_device = 16,
  timer_device = 17,
  input_device = 18,
  socket_device = 19,
  crypto_device = 20,
  signal_distribution_module = 21,
  pstore_device = 22,
  IOMMU_device = 23,
  memory_device = 24,
};

enum class status_field {
  ACKNOWLEDGE = 1,
  DRIVER = 2,
  DRIVER_OK = 4,
  FEATURES_OK = 8,
  DEVICE_NEEDS_RESET = 64,
  FAILED = 128,
};

enum class virtq_desc_flag : uint16_t {
  NONE = 0,
  VIRTQ_DESC_F_NEXT = 1,
  VIRTQ_DESC_F_WRITE = 2,
  VIRTQ_DESC_F_INDIRECT = 4,
};

struct virtq_desc {
  uint64_t addr;
  uint32_t len;
  virtq_desc_flag flags;
  uint16_t next;
};

enum class virtq_avail_flag : uint16_t {
  NONE = 0,
  VIRTQ_AVAIL_F_NO_INTERRUPT = 1,
};

struct virtq_avail {
  virtq_avail_flag flags; // always zero
  uint16_t idx;   // driver will write ring[idx] next
  uint16_t ring[queue_buffer_size]; // descriptor numbers of chain heads
  uint16_t used_event; // only if VIRTIO_F_EVENT_IDX
};

enum class virtq_used_flag : uint16_t {
  NONE = 0,
  VIRTQ_USED_F_NO_NOTIFY = 1,
};

/* le32 is used here for ids for padding reasons. */
struct virtq_used_elem {
  /* Index of start of used descriptor chain. */
  uint32_t id;
  /* Total length of the descriptor chain which was used (written to) */
  uint32_t len;
};

struct virtq_used {
  virtq_used_flag flags;
  uint16_t idx;
  virtq_used_elem ring[queue_buffer_size];
  uint16_t avail_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct virt_queue {
  virtq_desc desc[queue_buffer_size];
  virtq_avail avail;
  uint8_t reserved[memory_layout::PGSIZE - sizeof(desc) - sizeof(avail)];
  virtq_used used;
};

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

// device-independent feature bits
enum class feature_bit : uint64_t {
  VIRTIO_F_NOTIFY_ON_EMPTY = 1uL << 24,
  VIRTIO_F_ANY_LAYOUT = 1uL << 27,
  VIRTIO_F_RING_INDIRECT_DESC = 1uL << 28,
  VIRTIO_F_RING_EVENT_IDX = 1uL << 29,
  UNUSED = 1uL << 30,
  VIRTIO_F_VERSION_1 = 1uL << 32,
  VIRTIO_F_ACCESS_PLATFORM = 1uL << 33,
  VIRTIO_F_RING_PACKED = 1uL << 34,
  VIRTIO_F_IN_ORDER = 1uL << 35,
  VIRTIO_F_ORDER_PLATFORM = 1uL << 36,
  VIRTIO_F_SR_IOV = 1uL << 37,
  VIRTIO_F_NOTIFICATION_DATA = 1uL << 38,
};

enum class Operation : uint8_t {
  read = 1,
  write = 2,
};

struct MetaData {
  std::array<uint8_t, block_size> buf;
  uint64_t block_index;
};

class Device {
 public:
  Device(riscv::plic::irq e);
  virtual device_id GetDeviceId() = 0;
  virtual std::array<Operation, 64> GetSupportedOperation() = 0;
  virtual void ProcessInterrupt() = 0;

 protected:
  int AllocDesc();
  void FreeDesc(uint32_t desc_index);
  void FreeDesc(std::initializer_list<uint32_t> desc_list);

  uint8_t bit_map_[queue_buffer_size / 8] = {0};
  virt_queue* queue = nullptr;
  riscv::plic::irq irq_ = riscv::plic::irq::NONE;
  struct InternalData {
    volatile bool waiting_flag;
    uint8_t status = 0;
  };
  InternalData internal_data_[queue_buffer_size];
};

class BlockDevice : public Device {
 public:
  BlockDevice(uint64_t virtio_addr, riscv::plic::irq e);
  bool Init();
  bool Operate(Operation op, MetaData* meta_data);

  device_id GetDeviceId() override {
    return device_id::block_device;
  }

  std::array<Operation, 64> GetSupportedOperation() override {
    return {Operation::read, Operation::write};
  }

  void ProcessInterrupt() override;

 private:
  bool Validate();
  bool Alloc3Desc(std::array<uint32_t, 3>* descs);

  blk::virtio_blk_req blk_req[queue_buffer_size];
  uint64_t addr_ = 0;
  uint64_t capacity_ = 0;
  uint32_t last_seen_used_idx_ = 0;
};

}  // namespace virtio
}  // namespace driver


#endif  // DRIVER_VIRTIO_H
