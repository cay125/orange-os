#pragma once

#include "driver/virtio.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include <array>
#include <type_traits>

namespace driver {
namespace virtio {

template<typename... T>
class mmio_transport {
 public:
  mmio_transport(Device* device, virt_queue* queue, uint64_t addr, T... reqs) : device_(device), desc_index_(0), queue_(queue), addr_(addr) {
    bool ret = device_->AllocMultiDesc(&descs_);
    if (!ret) {
      kernel::printf("mmio_transport: alloc desc failed\n");
      return;
    }
    desc_alloc_succ = true;
    request(reqs...);
  }

  void set_indirect_flag(uint32_t desc_index) {
    queue_->desc[desc_index].flags |= virtq_desc_flag::VIRTQ_DESC_F_INDIRECT;
  }

  void trigger_notify() {
    if (!desc_alloc_succ) {
      return;
    }
    kernel::CriticalGuard guard(&device_->lk_);
    queue_->avail.ring[queue_->avail.idx % queue_buffer_size] = descs_[0];
    __sync_synchronize();
    queue_->avail.idx += 1;
    device_->internal_data_[descs_[0]].waiting_flag = true;
    __sync_synchronize();
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNotify, 0);
  }

  void wait_complete() {
    if (!desc_alloc_succ) {
      return;
    }
    kernel::CriticalGuard guard(&device_->lk_);
    while (device_->internal_data_[descs_[0]].waiting_flag) {
      kernel::Schedueler::Instance()->Sleep(&device_->channel_, &device_->lk_);
    }
    for (auto desc : descs_) {
      device_->FreeDesc(desc);
    }
  }

 private:
  template <class T1, class... T2, std::enable_if_t<std::is_pointer_v<T1>, bool> = true>
  void request(T1 req, T2... reqs) {
    queue_->desc[descs_[desc_index_]].addr = reinterpret_cast<uint64_t>(req);
    queue_->desc[descs_[desc_index_]].len = sizeof(*req);
    queue_->desc[descs_[desc_index_]].flags = virtq_desc_flag::VIRTQ_DESC_F_NEXT;
    if (!std::is_const_v<std::remove_pointer_t<T1>>) {
      queue_->desc[descs_[desc_index_]].flags |= virtq_desc_flag::VIRTQ_DESC_F_WRITE;
    }
    queue_->desc[descs_[desc_index_]].next = descs_[desc_index_ + 1];
    desc_index_ += 1;
    request(reqs...);
  }

  template <class REQ, std::enable_if_t<std::is_pointer_v<REQ>, bool> = true>
  void request(REQ req) {
    queue_->desc[descs_[desc_index_]].addr = reinterpret_cast<uint64_t>(req);
    queue_->desc[descs_[desc_index_]].len = sizeof(*req);
    queue_->desc[descs_[desc_index_]].flags = virtq_desc_flag::NONE;
    if (!std::is_const_v<std::remove_pointer_t<REQ>>) {
      queue_->desc[descs_[desc_index_]].flags |= virtq_desc_flag::VIRTQ_DESC_F_WRITE;
    }
    queue_->desc[descs_[desc_index_]].next = 0;
  }

  Device* device_;
  int desc_index_;
  bool desc_alloc_succ = false;
  std::array<uint32_t, sizeof...(T)> descs_;
  virt_queue* queue_;
  uint64_t addr_;
};

}
}