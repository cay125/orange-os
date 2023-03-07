#include "driver/virtio.h"

#include "arch/riscv_isa.h"
#include "kernel/extern_controller.h"
#include "kernel/virtual_memory.h"
#include "kernel/utils.h"
#include "kernel/printf.h"
#include "lib/string.h"

namespace driver {
namespace virtio {

int Device::AllocDesc() {
  for (uint32_t i = 0; i < queue_buffer_size; ++i) {
    if ((bit_map_[i / 8] & (1u << (i % 8))) == 0) {
      bit_map_[i / 8] |= 1u << (i % 8);
      return i;      
    }
  }
  return -1;
}

Device::Device(riscv::plic::irq e) : irq_(e) {
  kernel::ExternController::Instance()->Register(e, this);
}

void Device::FreeDesc(uint32_t desc_index) {
  if (desc_index >= queue_buffer_size) kernel::panic();
  if ((bit_map_[desc_index / 8] & (desc_index % 8)) == 0) kernel::panic();
  bit_map_[desc_index / 8] &= ~(1 << (desc_index % 8));
  memset(queue->desc + desc_index, 0, sizeof(virtq_desc));
}

void Device::FreeDesc(std::initializer_list<uint32_t> desc_list) {
  for (auto desc : desc_list) {
    FreeDesc(desc);
  }
}

BlockDevice::BlockDevice(uint64_t virtio_addr, riscv::plic::irq e) : Device(e), addr_(virtio_addr) {

}

bool BlockDevice::Init() {
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
  queue = reinterpret_cast<virt_queue*>(page);

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

bool BlockDevice::Alloc3Desc(std::array<uint32_t, 3>* descs) {
  (*descs)[0] = AllocDesc();
  if ((*descs)[0] < 0) {
    return false;
  }
  (*descs)[1] = AllocDesc();
  if ((*descs)[1] < 0) {
    FreeDesc({(*descs)[0]});
    return false;
  }
  (*descs)[2] = AllocDesc();
  if ((*descs)[2] < 0) {
    FreeDesc({(*descs)[0], (*descs)[1]});
    return false;
  }
  return true;
}

bool BlockDevice::Operate(Operation op, MetaData* meta_data) {
  if (op != Operation::read && op != Operation::write) {
    return false;
  }
  std::array<uint32_t, 3> descs;
  if (!Alloc3Desc(&descs)) {
    return false;
  }
  auto* req = &blk_req[descs[0]];
  if (op == Operation::read) {
    req->type = blk::req_type::VIRTIO_BLK_T_IN;
  } else if (op == Operation::write) {
    req->type = blk::req_type::VIRTIO_BLK_T_OUT;
  }
  req->sector = meta_data->block_index;
  queue->desc[descs[0]].addr = reinterpret_cast<uint64_t>(req);
  queue->desc[descs[0]].len = sizeof(blk::virtio_blk_req);
  queue->desc[descs[0]].flags = virtq_desc_flag::VIRTQ_DESC_F_NEXT;
  queue->desc[descs[0]].next = descs[1];

  queue->desc[descs[1]].addr = reinterpret_cast<uint64_t>(meta_data->buf.data());
  queue->desc[descs[1]].len = block_size;
  queue->desc[descs[1]].flags = virtq_desc_flag::VIRTQ_DESC_F_NEXT;
  if (op == Operation::read) {
    queue->desc[descs[1]].flags |= virtq_desc_flag::VIRTQ_DESC_F_WRITE;
  }
  queue->desc[descs[1]].next = descs[2];

  internal_data_[descs[0]].status = 0xff;
  queue->desc[descs[2]].addr = reinterpret_cast<uint64_t>(&internal_data_[descs[0]].status);
  queue->desc[descs[2]].len = 1;
  queue->desc[descs[2]].flags = virtq_desc_flag::VIRTQ_DESC_F_WRITE;
  queue->desc[descs[2]].next = 0;

  queue->avail.ring[queue->avail.idx % queue_buffer_size] = descs[0];

  __sync_synchronize();

  queue->avail.idx += 1;

  internal_data_[descs[0]].waiting_flag = true;

  __sync_synchronize();

  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNotify, 0);

  while (internal_data_[descs[0]].waiting_flag);

  return true;
}

void BlockDevice::ProcessInterrupt() {
  auto interrupt_status = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::InterruptStatus);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::InterruptACK, interrupt_status & 0x3);
  __sync_synchronize();
  while (last_seen_used_idx_ != queue->used.idx) {
    __sync_synchronize();
    auto& element = queue->used.ring[last_seen_used_idx_ % queue_buffer_size];
    internal_data_[element.id].waiting_flag = false;
    last_seen_used_idx_ += 1;
  }
}

}  // namespace virtio
}  // namespace driver