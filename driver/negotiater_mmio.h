#pragma once

#include "driver/virtio.h"
#include "kernel/printf.h"
#include "kernel/virtual_memory.h"
#include <algorithm>
#include <array>
#include <initializer_list>
#include <type_traits>
namespace driver {
namespace virtio {

template <class F, size_t N, size_t M>
class mmio_negotiater {
 public:
  mmio_negotiater(DeviceViaMMIO* device, feature_bit const (&independent_feat)[N] = {}, F const (&spec_feat)[M] = {}) : device_(device), addr_(device->addr_), has_init_(false) {
    std::copy(independent_feat, independent_feat + N, expected_independ_feature_.begin());
    std::copy(spec_feat, spec_feat + M, expected_spec_feature_.begin());
  }

  bool init(int queue_num) {
    if (has_init_) {
      kernel::printf("negotiater_mmio: duplicated init device, addr: %#x", addr_);
      return false;
    }
    has_init_ = true;

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
    uint32_t selected_feat = 0;
    for (size_t i = 0; i < expected_independ_feature_.size(); ++i) {
      if (feature_bits_low & expected_independ_feature_[i]) {
        has_feature_[i] = true;
        selected_feat |= expected_independ_feature_[i];
      }
    }
    for (size_t i = 0; i < expected_spec_feature_.size(); ++i) {
      if (feature_bits_low & expected_spec_feature_[i]) {
        has_feature_[i + N] = true;
        selected_feat |= expected_spec_feature_[i];
      }
    }
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DriverFeaturesSel, 0);
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::DriverFeatures, selected_feat);

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

    for (int queue_index = 0; queue_index < queue_num; ++queue_index) {
      MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::QueueSel, queue_index);
      if (MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueuePFN) != 0) {
        kernel::printf("[virtio] device: %#x queue_fpn[%d] is not zero, setting failed\n", addr_, queue_index);
        return false;
      }
      auto max_queue_buffer = MEMORY_MAPPED_IO_R_WORD(addr_ + mmio_addr::QueueNumMax);
      kernel::printf("[virtio] device: %#x max queue[%d] buffer: %d\n", addr_, queue_index, max_queue_buffer);
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
      device_->queue[queue_index] = reinterpret_cast<virt_queue*>(page);
    }

    // OS finish device setting
    status |= status_field::DRIVER_OK;
    MEMORY_MAPPED_IO_W_WORD(addr_ + mmio_addr::Status, status);

    return true;
  }

  bool contain(feature_bit feat) {
    for (size_t i = 0; i < expected_independ_feature_.size(); ++i) {
      if (expected_independ_feature_ == feat) {
        return has_feature_[i];
      }
    }
    return false;
  }

  bool contain(F feat) requires (!std::is_same_v<F, feature_bit>) {
    for (size_t i = 0; i < expected_spec_feature_.size(); ++i) {
      if (expected_spec_feature_[i] == feat) {
        return has_feature_[i + N];
      }
    }
    return false;
  }

 private:
  DeviceViaMMIO* device_;
  uint64_t addr_;
  bool has_init_;
  std::array<feature_bit, N> expected_independ_feature_;
  std::array<F, M> expected_spec_feature_;
  bool has_feature_[N + M];
};

mmio_negotiater(DeviceViaMMIO*) -> mmio_negotiater<feature_bit, 0, 0>;

template <size_t N>
mmio_negotiater(DeviceViaMMIO*, feature_bit const (&) [N]) -> mmio_negotiater<feature_bit, N, 0>;

}
}
