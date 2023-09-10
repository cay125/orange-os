#pragma once

#include "driver/virtio.h"
#include "lib/types.h"
#include <array>

namespace driver {
namespace virtio {

namespace gpu {

enum class feature_bit : uint64_t {
  VIRTIO_GPU_F_VIRGL = 0,
  VIRTIO_GPU_F_EDID = 1,
};

struct virtio_gpu_config {
  uint32_t events_read;
  uint32_t events_clear;
  uint32_t num_scanouts;
  uint32_t reserved;
};

enum class virtio_gpu_ctrl_type : uint32_t {
  /* 2d commands */
  VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
  VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
  VIRTIO_GPU_CMD_RESOURCE_UNREF,
  VIRTIO_GPU_CMD_SET_SCANOUT,
  VIRTIO_GPU_CMD_RESOURCE_FLUSH,
  VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
  VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
  VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
  VIRTIO_GPU_CMD_GET_CAPSET_INFO,
  VIRTIO_GPU_CMD_GET_CAPSET,
  VIRTIO_GPU_CMD_GET_EDID,
  /* cursor commands */
  VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
  VIRTIO_GPU_CMD_MOVE_CURSOR,
  /* success responses */
  VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
  VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
  VIRTIO_GPU_RESP_OK_CAPSET_INFO,
  VIRTIO_GPU_RESP_OK_CAPSET,
  VIRTIO_GPU_RESP_OK_EDID,
  /* error responses */
  VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
  VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
  VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

constexpr uint32_t VIRTIO_GPU_FLAG_FENCE = (1 << 0);

struct virtio_gpu_ctrl_hdr {
  uint32_t type;
  uint32_t flags;
  uint64_t fence_id;
  uint32_t ctx_id;
  uint32_t padding;
};

struct virtio_gpu_rect {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
};

#define VIRTIO_GPU_MAX_SCANOUTS 16
struct virtio_gpu_resp_display_info {
  virtio_gpu_ctrl_hdr hdr;
  struct virtio_gpu_display_one {
    virtio_gpu_rect r;
    uint32_t enabled;
    uint32_t flags;
  } pmodes[VIRTIO_GPU_MAX_SCANOUTS];
};

enum class virtio_gpu_formats : uint32_t {
  VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
  VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
  VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
  VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,
  VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
  VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,
  VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
  VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};

struct virtio_gpu_resource_create_2d {
  virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t format;
  uint32_t width;
  uint32_t height;
};

struct virtio_gpu_resource_attach_backing {
  virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t nr_entries;
};

struct virtio_gpu_resource_detach_backing {
  virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t padding;
};

struct virtio_gpu_mem_entry {
  uint64_t addr;
  uint32_t length;
  uint32_t padding;
};

struct virtio_gpu_set_scanout {
  virtio_gpu_ctrl_hdr hdr;
  virtio_gpu_rect r;
  uint32_t scanout_id;
  uint32_t resource_id;
};

struct virtio_gpu_transfer_to_host_2d {
  virtio_gpu_ctrl_hdr hdr;
  virtio_gpu_rect r;
  uint64_t offset;
  uint32_t resource_id;
  uint32_t padding;
};

struct virtio_gpu_resource_flush {
  virtio_gpu_ctrl_hdr hdr;
  virtio_gpu_rect r;
  uint32_t resource_id;
  uint32_t padding;
};

struct virtio_gpu_resource_unref {
  virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t padding;
};

struct virtio_gpu_cursor_pos {
  uint32_t scanout_id;
  uint32_t x;
  uint32_t y;
  uint32_t padding;
};

struct virtio_gpu_update_cursor {
  virtio_gpu_ctrl_hdr hdr;
  virtio_gpu_cursor_pos pos;
  uint32_t resource_id;
  uint32_t hot_x;
  uint32_t hot_y;
  uint32_t padding;
};

constexpr uint32_t cursor_image_size = 64 * 64 * 4;
using CursorImage = std::array<uint8_t, cursor_image_size>;

}

class GPUDevice : public Device {
 public:
  GPUDevice();
  bool Init(uint64_t virtio_addr) override;
  device_id GetDeviceId() override;
  void ProcessInterrupt() override;

  gpu::virtio_gpu_resp_display_info GetDisplayInfo();
  bool SetupFramebuffer(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint64_t addr, size_t size);
  bool SetupCursor(const gpu::CursorImage* cursor_image, uint32_t pos_x, uint32_t pos_y, uint32_t hot_x, uint32_t hot_y);
  void MoveCursor(uint32_t pos_x, uint32_t pos_y);
  void Flush();
  bool Drop();
  virtq_desc* mutable_indirect_desc();

 private:
  void Create2dResource(uint32_t resource_id, uint32_t width, uint32_t height, uint32_t flags = 0);
  void ResourceAttachBacking(uint32_t resource_id, uint64_t addr, size_t size, uint32_t flags = 0);
  void ResourceDetachBacking(uint32_t resource_id);
  void SetScanout(const gpu::virtio_gpu_rect& rect, uint32_t scanout_id, uint32_t resource_id);
  void TransferTo2D(const gpu::virtio_gpu_rect& rect, uint64_t offset, uint32_t resource_id, uint32_t flags = 0);
  void ResourceFlush(const gpu::virtio_gpu_rect& rect, uint32_t resource_id);
  void ResourceDestroy(uint32_t resource_id);
  bool Validate();
  void UpdateCursor(uint32_t resource_id, uint32_t scanout_id, uint32_t pos_x, uint32_t pos_y, uint32_t hot_x, uint32_t hot_y, bool is_move = false);

  uint64_t addr_ = 0;
  gpu::virtio_gpu_rect screen_rect_{};
  // The mouse cursor image must be 64x64 in size
  const gpu::virtio_gpu_rect cursor_rect_{0, 0, 64, 64};
  uint32_t resource_id_frame_buffer_ = 0;
  uint32_t resource_id_cursor_ = 0;
  const uint32_t scanout_id_ = 0;
  gpu::virtio_gpu_config config_{};
  virtq_desc indirect_desc_[512];
};

}
}