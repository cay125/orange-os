#pragma once

#include "driver/virtio.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include <array>
#include <type_traits>
#include <utility>

template<typename T>
struct is_pair_t : std::false_type {};

template<typename T1, typename T2>
struct is_pair_t<std::pair<T1, T2>> : std::true_type {};

namespace driver {
namespace virtio {

template<typename... T>
class mmio_transport {
 public:
  mmio_transport(Device* device, int queue_index, uint64_t addr, T... reqs) : device_(device), desc_index_(0), queue_index_(queue_index), queue_(device_->queue[queue_index]), addr_(addr) {
    bool ret = device_->AllocMultiDesc(&descs_, queue_index);
    if (!ret) {
      kernel::printf("mmio_transport: alloc desc failed\n");
      return;
    }
    desc_alloc_succ = true;
    request(reqs...);
  }

  mmio_transport(const mmio_transport&) = delete;
  mmio_transport& operator= (const mmio_transport&) = delete;

  void set_indirect_flag(uint32_t desc_index) {
    queue_->desc[desc_index].flags |= virtq_desc_flag::VIRTQ_DESC_F_INDIRECT;
  }

  void trigger_notify() {
    if (!desc_alloc_succ) {
      return;
    }
    kernel::CriticalGuard guard(&device_->lk_[queue_index_]);
    queue_->avail.ring[queue_->avail.idx % queue_buffer_size] = descs_[0];
    __sync_synchronize();
    queue_->avail.idx += 1;
    device_->internal_data_[descs_[0]].waiting_flag = true;
    __sync_synchronize();
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueNotify, queue_index_);
  }

  void wait_complete() {
    if (!desc_alloc_succ) {
      return;
    }
    kernel::CriticalGuard guard(&device_->lk_[queue_index_]);
    while (device_->internal_data_[descs_[0]].waiting_flag) {
      kernel::Schedueler::Instance()->Sleep(&device_->channel_, &device_->lk_[queue_index_]);
    }
    for (auto desc : descs_) {
      device_->FreeDesc(desc, queue_index_);
    }
  }

  std::array<uint32_t, sizeof...(T)> get_descs() const {
    return descs_;
  }

 private:
  template <class T1, class... T2, std::enable_if_t<std::is_pointer_v<T1>, bool> = true>
  void request(T1 req, T2... reqs) {
    virtq_desc_flag flag = std::is_const_v<std::remove_pointer_t<T1>> ? virtq_desc_flag::VIRTQ_DESC_F_NEXT : virtq_desc_flag::VIRTQ_DESC_F_NEXT | virtq_desc_flag::VIRTQ_DESC_F_WRITE;
    if constexpr (!is_pair_t<std::decay_t<std::remove_pointer_t<T1>>>::value) {
      fill_desc(&queue_->desc[descs_[desc_index_]], req, flag, descs_[desc_index_ + 1]);
    } else {
      fill_desc(&queue_->desc[descs_[desc_index_]], req->first, req->second, flag, descs_[desc_index_ + 1]);
    }
    desc_index_ += 1;
    request(reqs...);
  }

  template <class REQ, std::enable_if_t<std::is_pointer_v<REQ>, bool> = true>
  void request(REQ req) {
    virtq_desc_flag flag = std::is_const_v<std::remove_pointer_t<REQ>> ? virtq_desc_flag::NONE : virtq_desc_flag::VIRTQ_DESC_F_WRITE;
    if constexpr (!is_pair_t<std::decay_t<std::remove_pointer_t<REQ>>>::value) {
      fill_desc(&queue_->desc[descs_[desc_index_]], req, flag, 0);
    } else {
      fill_desc(&queue_->desc[descs_[desc_index_]], req->first, req->second, flag, 0);
    }
  }

  Device* device_;
  int desc_index_;
  bool desc_alloc_succ = false;
  std::array<uint32_t, sizeof...(T)> descs_;
  int queue_index_;
  virt_queue* queue_;
  uint64_t addr_;
};

}
}