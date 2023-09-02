#include "driver/transport_mmio.h"
#include "driver/virtio_gpu.h"
#include "driver/virtio.h"
#include "kernel/config/memory_layout.h"
#include "kernel/printf.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/common.h"
#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

namespace driver {
namespace virtio {

GPUDevice::GPUDevice() {

}

bool GPUDevice::Validate() {
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::MagicValue) != magic_num) return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::Version) != device_version::Legacy) return false;
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::DeviceID) != device_id::GPU_device) return false;
  kernel::printf("[virtio] device: %#x vendor id: %#x\n", addr_, MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::VendorID));
  return true;
}

bool GPUDevice::Init(uint64_t virtio_addr) {
  addr_ = virtio_addr;
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

  config_ = *reinterpret_cast<gpu::virtio_gpu_config*>(addr_ + mmio_addr::Config);
  kernel::printf("[virtio] device: %#x events_read: %d, num_scanouts: %d\n", addr_, config_.events_read, config_.num_scanouts);

  // set control virt-queue
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueSel, 0);
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueuePFN) != 0) {
    kernel::printf("[virtio] device: %#x queue_fpn[0] is not zero, setting failed\n", addr_);
    return false;
  }
  auto max_queue_buffer = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueueNumMax);
  kernel::printf("[virtio] device: %#x max queue[0] buffer: %d\n", addr_, max_queue_buffer);
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

  // set cursor virt-queue
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueSel, 1);
  if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueuePFN) != 0) {
    kernel::printf("[virtio] device: %#x queue_fpn[1] is not zero, setting failed\n", addr_);
    return false;
  }
  max_queue_buffer = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueueNumMax);
  kernel::printf("[virtio] device: %#x max queue[1] buffer: %d\n", addr_, max_queue_buffer);
  if (max_queue_buffer < queue_buffer_size) {
    return false;
  }
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNum, queue_buffer_size);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueAlign, memory_layout::PGSIZE);

  page = kernel::VirtualMemory::Instance()->AllocContinuousPage(2);
  if (!page) {
    kernel::printf("[virtio] device: %#x alloc memory page failed\n", addr_);
    return false;
  }
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueuePFN, reinterpret_cast<uint64_t>(page) / memory_layout::PGSIZE);
  queue[1] = reinterpret_cast<virt_queue*>(page);

  // OS finish device setting
  status |= status_field::DRIVER_OK;
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

  return true;
}


bool GPUDevice::Operate(Operation op, MetaData* meta_data) {
  return true;
}

device_id GPUDevice::GetDeviceId() {
  return device_id::GPU_device;
}

std::array<Operation, 64> GPUDevice::GetSupportedOperation() {
  return {};
}

void GPUDevice::ProcessInterrupt() {
  kernel::CriticalGuard guard(&lk_);
  auto interrupt_status = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::InterruptStatus);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::InterruptACK, interrupt_status & 0x3);
  __sync_synchronize();
  while (last_seen_used_idx_ != queue[0]->used.idx) {
    __sync_synchronize();
    auto& element = queue[0]->used.ring[last_seen_used_idx_ % queue_buffer_size];
    internal_data_[element.id].waiting_flag = false;
    last_seen_used_idx_ += 1;
    kernel::Schedueler::Instance()->Wakeup(&channel_);
  }
}

#define CHECK_TYPE(hdr, target_type) check_type(__PRETTY_FUNCTION__, hdr, target_type);

static void check_type(const char* fun_name, const gpu::virtio_gpu_ctrl_hdr* hdr, gpu::virtio_gpu_ctrl_type target_type) {
  if (hdr->type != lib::common::literal(target_type)) {
    kernel::panic("%s: gpu device resp error: %#x", fun_name, hdr->type);
  }
}

gpu::virtio_gpu_resp_display_info GPUDevice::GetDisplayInfo() {
  gpu::virtio_gpu_ctrl_hdr header{};
  header.type = lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_GET_DISPLAY_INFO);
  gpu::virtio_gpu_resp_display_info display_info{};
  mmio_transport transport(this, queue[0], addr_, &std::as_const(header), &display_info);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&display_info.hdr, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_DISPLAY_INFO);
  return display_info;
}

void GPUDevice::Create2dResource(uint32_t resource_id, uint32_t width, uint32_t height) {
  gpu::virtio_gpu_resource_create_2d create_info{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_CREATE_2D)},
    resource_id,
    lib::common::literal(gpu::virtio_gpu_formats::VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM),
    width,
    height
  };
  gpu::virtio_gpu_ctrl_hdr header;
  mmio_transport transport(this, queue[0], addr_, &std::as_const(create_info), &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

bool GPUDevice::SetupFramebuffer(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint64_t addr, size_t size) {
  if (scanout_id > (config_.num_scanouts - 1)) {
    kernel::printf("SetupFramebuffer: scanout_id exceed limit, %d vs %d\n", scanout_id, config_.num_scanouts);
    return false;
  }
  rect_ = rect;
  Create2dResource(resource_id_frame_buffer_, rect.width, rect.height);
  ResourceAttachBacking(resource_id_frame_buffer_, addr, size);
  SetScanout(rect, scanout_id_, resource_id_frame_buffer_);
  return true;
}

void GPUDevice::ResourceAttachBacking(uint32_t resource_id, uint64_t addr, size_t size) {
  if (size == 0) {
    return;
  }
  uint32_t page_num = (size + memory_layout::PGSIZE - 1) / memory_layout::PGSIZE;
  const gpu::virtio_gpu_resource_attach_backing backing{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING)},
    resource_id,
    page_num,
  };
  fill_desc(&indirect_desc_[0], &backing, virtq_desc_flag::VIRTQ_DESC_F_NEXT, 1);

  uint32_t need_entry_mem = (sizeof(gpu::virtio_gpu_mem_entry) * page_num + memory_layout::PGSIZE - 1) / memory_layout::PGSIZE;
  auto* entrys = reinterpret_cast<gpu::virtio_gpu_mem_entry*>(kernel::VirtualMemory::Instance()->AllocContinuousPage(need_entry_mem));

  std::generate(entrys, entrys + page_num, [addr](){
    static size_t index = 0;
    auto* process = kernel::Schedueler::Instance()->ThisProcess();
    return gpu::virtio_gpu_mem_entry{
      kernel::VirtualMemory::Instance()->VAToPA(process->page_table, index++ * memory_layout::PGSIZE + addr),
      memory_layout::PGSIZE,
      0
    };
  });

  std::generate(indirect_desc_ + 1, indirect_desc_ + 1 + page_num, [entrys](){
    static uint16_t index = 0;
    return virtq_desc{
      reinterpret_cast<uint64_t>(entrys + index),
      sizeof(gpu::virtio_gpu_mem_entry),
      virtq_desc_flag::VIRTQ_DESC_F_NEXT,
      static_cast<uint16_t>((index++) + 2)
    };
  });

  gpu::virtio_gpu_ctrl_hdr header;
  fill_desc(&indirect_desc_[page_num + 1], &header, virtq_desc_flag::VIRTQ_DESC_F_WRITE, 0);
  mmio_transport transport(this, queue[0], addr_, &std::as_const(indirect_desc_));
  transport.set_indirect_flag(0);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);

  for (uint32_t i =0; i < need_entry_mem; i++) {
    kernel::VirtualMemory::Instance()->FreePage((uint64_t*)(((uint64_t)entrys) + i * memory_layout::PGSIZE));
  }
}

void GPUDevice::SetScanout(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint32_t resource_id) {
  const gpu::virtio_gpu_set_scanout set_scanout{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_SET_SCANOUT)},
    rect,
    scanout_id,
    resource_id
  };
  gpu::virtio_gpu_ctrl_hdr header;
  mmio_transport transport(this, queue[0], addr_, &set_scanout, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

void GPUDevice::Flush() {
  TransferTo2D(rect_, 0, resource_id_frame_buffer_);
  ResourceFlush(rect_, resource_id_frame_buffer_);
}

void GPUDevice::TransferTo2D(const gpu::virtio_gpu_rect& rect, uint64_t offset, uint32_t resource_id) {
  const gpu::virtio_gpu_transfer_to_host_2d transfer{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D)},
    rect,
    offset,
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header;
  mmio_transport transport(this, queue[0], addr_, &transfer, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

void GPUDevice::ResourceFlush(const gpu::virtio_gpu_rect& rect, uint32_t resource_id) {
  const gpu::virtio_gpu_resource_flush resource_flush{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_FLUSH)},
    rect,
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header;
  mmio_transport transport(this, queue[0], addr_, &resource_flush, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

virtq_desc* GPUDevice::mutable_indirect_desc() {
  return indirect_desc_;
}

}
}