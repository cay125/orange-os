#include "driver/negotiater_mmio.h"
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

GPUDevice::GPUDevice() : DeviceViaMMIO(0) {

}

bool GPUDevice::Init(uint64_t virtio_addr) {
  addr_ = virtio_addr;
  if (!Validate()) {
    return false;
  }
  mmio_negotiater negotiater(
    this,
    {feature_bit::VIRTIO_F_NOTIFY_ON_EMPTY, feature_bit::VIRTIO_F_ANY_LAYOUT, feature_bit::VIRTIO_F_RING_INDIRECT_DESC},
    {gpu::feature_bit::VIRTIO_GPU_F_EDID}
  );
  bool ret = negotiater.init(2);
  if (!ret) {
    return false;
  }

  config_ = *reinterpret_cast<gpu::virtio_gpu_config*>(addr_ + mmio_addr::Config);
  kernel::printf("[virtio] device: %#x events_read: %d, num_scanouts: %d\n", addr_, config_.events_read, config_.num_scanouts);

  return true;
}

device_id GPUDevice::GetDeviceId() {
  return device_id::GPU_device;
}

void GPUDevice::ProcessInterrupt() {
  kernel::CriticalGuard guard1(&lk_[0]);
  kernel::CriticalGuard guard2(&lk_[1]);
  auto interrupt_status = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::InterruptStatus);
  MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::InterruptACK, interrupt_status & 0x3);
  __sync_synchronize();
  for (int i = 0; i < 2; ++i) {
    while (last_seen_used_idx_[i] != queue[i]->used.idx) {
      __sync_synchronize();
      auto& element = queue[i]->used.ring[last_seen_used_idx_[i] % queue_buffer_size];
      internal_data_[element.id].waiting_flag = false;
      last_seen_used_idx_[i] += 1;
      kernel::Schedueler::Instance()->Wakeup(&channel_);
    }
  }
}

#define CHECK_TYPE(hdr, target_type) check_type(__PRETTY_FUNCTION__, hdr, target_type);

static void check_type(const char* fun_name, const gpu::virtio_gpu_ctrl_hdr* hdr, gpu::virtio_gpu_ctrl_type target_type) {
  if (hdr->type != lib::common::literal(target_type)) {
    kernel::panic("%s: gpu device resp error: %#x", fun_name, hdr->type);
  }
}

#define CHECK_FENCE(hdr, target_fence_id) check_fence_id(__PRETTY_FUNCTION__, hdr, target_fence_id);

static void check_fence_id(const char* fun_name, const gpu::virtio_gpu_ctrl_hdr* hdr, uint64_t target_fence_id) {
  if (!(hdr->flags & gpu::VIRTIO_GPU_FLAG_FENCE) || hdr->fence_id != target_fence_id) {
    kernel::panic("%s: gpu device resp fence_id error", fun_name);
  }
}

gpu::virtio_gpu_resp_display_info GPUDevice::GetDisplayInfo() {
  gpu::virtio_gpu_ctrl_hdr header{};
  header.type = lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_GET_DISPLAY_INFO);
  gpu::virtio_gpu_resp_display_info display_info{};
  mmio_transport transport(this, 0, addr_, &std::as_const(header), &display_info);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&display_info.hdr, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_DISPLAY_INFO);
  return display_info;
}

void GPUDevice::Create2dResource(uint32_t resource_id, uint32_t width, uint32_t height, uint32_t flags) {
  gpu::virtio_gpu_resource_create_2d create_info{
    gpu::virtio_gpu_ctrl_hdr{
      lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_CREATE_2D),
      flags,
      (flags & gpu::VIRTIO_GPU_FLAG_FENCE) > 0 ? (uint64_t)&flags : 0
    },
    resource_id,
    lib::common::literal(gpu::virtio_gpu_formats::VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM),
    width,
    height
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &std::as_const(create_info), &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
  if (flags & gpu::VIRTIO_GPU_FLAG_FENCE) {
    CHECK_FENCE(&header, (uint64_t)&flags);
  }
}

bool GPUDevice::SetupFramebuffer(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint64_t addr, size_t size) {
  if (scanout_id > (config_.num_scanouts - 1)) {
    kernel::printf("SetupFramebuffer: scanout_id exceed limit, %d vs %d\n", scanout_id, config_.num_scanouts);
    return false;
  }
  screen_rect_ = rect;
  constexpr uint32_t resource_id_frame_buffer = 0xaabb;
  resource_id_frame_buffer_ = resource_id_frame_buffer;
  Create2dResource(resource_id_frame_buffer_, rect.width, rect.height);
  ResourceAttachBacking(resource_id_frame_buffer_, addr, size);
  SetScanout(rect, scanout_id_, resource_id_frame_buffer_);
  return true;
}

bool GPUDevice::SetupCursor(const gpu::CursorImage* cursor_image, uint32_t pos_x, uint32_t pos_y, uint32_t hot_x, uint32_t hot_y) {
  constexpr uint32_t resource_id_cursor = 0xccdd;
  resource_id_cursor_ = resource_id_cursor;
  Create2dResource(resource_id_cursor_, cursor_rect_.width, cursor_rect_.height, gpu::VIRTIO_GPU_FLAG_FENCE);
  ResourceAttachBacking(resource_id_cursor_, reinterpret_cast<uint64_t>(cursor_image->data()), gpu::cursor_image_size, gpu::VIRTIO_GPU_FLAG_FENCE);
  TransferTo2D(cursor_rect_, 0,resource_id_cursor_, gpu::VIRTIO_GPU_FLAG_FENCE);
  UpdateCursor(resource_id_cursor_, scanout_id_, pos_x, pos_y, hot_x, hot_y, false);
  return true;
}

void GPUDevice::UpdateCursor(uint32_t resource_id, uint32_t scanout_id, uint32_t pos_x, uint32_t pos_y, uint32_t hot_x, uint32_t hot_y, bool is_move) {
  const gpu::virtio_gpu_update_cursor cursor{
    gpu::virtio_gpu_ctrl_hdr{is_move ? lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_MOVE_CURSOR) : lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_UPDATE_CURSOR)},
    gpu::virtio_gpu_cursor_pos{
      scanout_id,
      pos_x,
      pos_y,
      0
    },
    resource_id,
    hot_x,
    hot_y,
    0
  };
  mmio_transport transport(this, 1, addr_, &cursor);
  transport.trigger_notify();
  transport.wait_complete();
}

void GPUDevice::MoveCursor(uint32_t pos_x, uint32_t pos_y) {
  UpdateCursor(resource_id_cursor_, scanout_id_, pos_x, pos_y, 0, 0, true);
}

void GPUDevice::ResourceAttachBacking(uint32_t resource_id, uint64_t addr, size_t size, uint32_t flags) {
  if (size == 0) {
    return;
  }
  auto aligned_gap = addr - kernel::VirtualMemory::AddrCastDown(addr);
  uint32_t page_num = (size + aligned_gap + memory_layout::PGSIZE - 1) / memory_layout::PGSIZE;
  const gpu::virtio_gpu_resource_attach_backing backing{
    gpu::virtio_gpu_ctrl_hdr{
      lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING),
      flags,
      (flags & gpu::VIRTIO_GPU_FLAG_FENCE) > 0 ? (uint64_t)&flags : 0
    },
    resource_id,
    page_num,
  };
  fill_desc(&indirect_desc_[0], &backing, virtq_desc_flag::VIRTQ_DESC_F_NEXT, 1);

  uint32_t need_entry_mem = (sizeof(gpu::virtio_gpu_mem_entry) * page_num + memory_layout::PGSIZE - 1) / memory_layout::PGSIZE;
  auto* entrys = reinterpret_cast<gpu::virtio_gpu_mem_entry*>(kernel::VirtualMemory::Instance()->AllocContinuousPage(need_entry_mem));

  std::generate(entrys, entrys + page_num, [size, addr]() mutable {
    auto* process = kernel::Schedueler::Instance()->ThisProcess();
    uint64_t remain_size = memory_layout::PGSIZE - addr % memory_layout::PGSIZE;
    uint64_t len = std::min(size, remain_size);
    gpu::virtio_gpu_mem_entry entry{
      kernel::VirtualMemory::Instance()->VAToPA(process->page_table, addr),
      static_cast<uint32_t>(len),
      0
    };
    size -= len;
    addr += len;
    return entry;
  });

  std::generate(indirect_desc_ + 1, indirect_desc_ + 1 + page_num, [entrys, index = 0]() mutable {
    return virtq_desc{
      reinterpret_cast<uint64_t>(entrys + index),
      sizeof(gpu::virtio_gpu_mem_entry),
      virtq_desc_flag::VIRTQ_DESC_F_NEXT,
      static_cast<uint16_t>((index++) + 2)
    };
  });

  gpu::virtio_gpu_ctrl_hdr header{};
  fill_desc(&indirect_desc_[page_num + 1], &header, virtq_desc_flag::VIRTQ_DESC_F_WRITE, 0);
  mmio_transport transport(this, 0, addr_, &std::as_const(indirect_desc_));
  transport.set_indirect_flag(0);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
  if (flags & gpu::VIRTIO_GPU_FLAG_FENCE) {
    CHECK_FENCE(&header, (uint64_t)&flags);
  }

  for (uint32_t i =0; i < need_entry_mem; i++) {
    kernel::VirtualMemory::Instance()->FreePage((uint64_t*)(((uint64_t)entrys) + i * memory_layout::PGSIZE));
  }
}

void GPUDevice::ResourceDetachBacking(uint32_t resource_id) {
  const gpu::virtio_gpu_resource_detach_backing detach{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING)},
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &detach, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

void GPUDevice::SetScanout(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint32_t resource_id) {
  const gpu::virtio_gpu_set_scanout set_scanout{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_SET_SCANOUT)},
    rect,
    scanout_id,
    resource_id
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &set_scanout, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

void GPUDevice::Flush() {
  TransferTo2D(screen_rect_, 0, resource_id_frame_buffer_);
  ResourceFlush(screen_rect_, resource_id_frame_buffer_);
}

bool GPUDevice::Drop() {
  if (resource_id_frame_buffer_ == 0 && resource_id_cursor_ == 0) {
    return false;
  }
  if (resource_id_frame_buffer_ > 0) {
    ResourceDetachBacking(resource_id_frame_buffer_);
    ResourceDestroy(resource_id_frame_buffer_);
    resource_id_frame_buffer_ = 0;
  }
  if (resource_id_cursor_ > 0) {
    ResourceDetachBacking(resource_id_cursor_);
    ResourceDestroy(resource_id_cursor_);
    resource_id_cursor_ = 0;
  }
  return true;
}

void GPUDevice::TransferTo2D(const gpu::virtio_gpu_rect& rect, uint64_t offset, uint32_t resource_id, uint32_t flags) {
  const gpu::virtio_gpu_transfer_to_host_2d transfer{
    gpu::virtio_gpu_ctrl_hdr{
      lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D),
      flags,
      (flags & gpu::VIRTIO_GPU_FLAG_FENCE) > 0 ? (uint64_t)&flags : 0
    },
    rect,
    offset,
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &transfer, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
  if (flags & gpu::VIRTIO_GPU_FLAG_FENCE) {
    CHECK_FENCE(&header, (uint64_t)&flags);
  }
}

void GPUDevice::ResourceFlush(const gpu::virtio_gpu_rect& rect, uint32_t resource_id) {
  const gpu::virtio_gpu_resource_flush resource_flush{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_FLUSH)},
    rect,
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &resource_flush, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

void GPUDevice::ResourceDestroy(uint32_t resource_id) {
  const gpu::virtio_gpu_resource_unref unref{
    gpu::virtio_gpu_ctrl_hdr{lib::common::literal(gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_CMD_RESOURCE_UNREF)},
    resource_id,
    0
  };
  gpu::virtio_gpu_ctrl_hdr header{};
  mmio_transport transport(this, 0, addr_, &unref, &header);
  transport.trigger_notify();
  transport.wait_complete();
  CHECK_TYPE(&header, gpu::virtio_gpu_ctrl_type::VIRTIO_GPU_RESP_OK_NODATA);
}

virtq_desc* GPUDevice::mutable_indirect_desc() {
  return indirect_desc_;
}

}
}