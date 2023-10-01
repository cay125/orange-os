#ifndef DRIVER_VIRTIO_H
#define DRIVER_VIRTIO_H

#include <array>
#include <initializer_list>

#include "driver/basic_device.h"
#include "kernel/config/memory_layout.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/lock/spin_lock.h"
#include "kernel/process.h"
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

template <class T>
void fill_desc(virtq_desc* desc, T* addr, virtq_desc_flag flag, uint16_t next) {
  desc->addr = reinterpret_cast<uint64_t>(addr);
  desc->len = sizeof(T);
  desc->flags = flag;
  if (flag & virtq_desc_flag::VIRTQ_DESC_F_NEXT) {
    desc->next = next;
  } else {
    desc->next = 0;
  }
}

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

constexpr int max_queue_num = 2;

template<class... T>
class mmio_transport;
template <class F, size_t N, size_t M>
class mmio_negotiater;
class Device : public BasicDevice {
  template<class... T>
  friend class mmio_transport;
 public:
  Device();
  virtual device_id GetDeviceId() = 0;

 protected:
  int AllocDesc(int queue_index);
  template<size_t N>
  bool AllocMultiDesc(std::array<uint32_t, N>* descs, int queue_index) {
    kernel::CriticalGuard guard(&lk_[queue_index]);
    for (size_t i = 0; i < descs->size(); ++i) {
      (*descs)[i] = AllocDesc(queue_index);
      if ((*descs)[i] < 0) {
        for (size_t j = 0; j < i; ++j) {
          FreeDesc((*descs)[j], queue_index);
        }
        return false;
      }
    }
    return true;
  }
  void FreeDesc(uint32_t desc_index, int queue_index);
  void FreeDesc(std::initializer_list<uint32_t> desc_list, int queue_index);

  uint8_t bit_map_[max_queue_num][queue_buffer_size / 8] = {{0}, {0}};
  virt_queue* queue[max_queue_num] = {nullptr};
  struct InternalData {
    volatile bool waiting_flag;
  };
  InternalData internal_data_[queue_buffer_size];
  uint32_t last_seen_used_idx_[max_queue_num] = {0};
  kernel::SpinLock lk_[max_queue_num];
  kernel::Channel channel_;
};

class DeviceViaMMIO : public Device {
  template <class F, size_t N, size_t M>
  friend class mmio_negotiater;
 public:
  DeviceViaMMIO(uint64_t addr) : Device(), addr_(addr) {

  }

 protected:
  bool Validate();

  uint64_t addr_;
};

}  // namespace virtio
}  // namespace driver


#endif  // DRIVER_VIRTIO_H
