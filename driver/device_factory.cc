#include "driver/device_factory.h"

#include <ranges>

#include "arch/riscv_plic.h"
#include "kernel/config/memory_layout.h"
#include "kernel/extern_controller.h"
#include "kernel/utils.h"
#include "lib/common.h"

namespace driver {

using lib::common::literal;

constexpr inline auto enum_range = [](auto front, auto back) {
  return std::views::iota(literal(front) + 1, literal(back)) | std::views::transform([](auto e) {
    return decltype(front)(e);
  });
};

struct DeviceInfo {
  uint64_t device_addr;
  riscv::plic::irq irq;
};

template <class T, std::size_t N>
struct EnumArray : std::array<T, N> {
  template <typename I>
  T& operator[](const I& i) {
    return std::array<T, N>::operator[](static_cast<std::underlying_type_t<I>>(i));
  }

  template <typename I>
  const T& operator[](const I& i) const {
    return std::array<T, N>::operator[](static_cast<std::underlying_type_t<I>>(i));
  }
};

EnumArray<DeviceInfo, static_cast<std::underlying_type_t<DeviceList>>(DeviceList::device_list_end)> device_info = {
  // begin
  DeviceInfo{0, riscv::plic::irq::NONE},
  // disko
  DeviceInfo{memory_layout::VIRTIO0, riscv::plic::irq::VIRTIO0_IRQ},
  // uart0
  DeviceInfo{memory_layout::UART0, riscv::plic::irq::UARRT0_IRQ},
};

void DeviceFactory::InitDevices() {
  for (const DeviceList device_enum : enum_range(DeviceList::device_list_begin, DeviceList::device_list_end)) {
    auto* device = GetDevice(device_enum);
    if (!device) {
      kernel::panic("Got empty device, index: %d", literal(device_enum));
    }
    if (!device->Init(device_info[device_enum].device_addr)) {
      kernel::panic("Init device failed, index: %d", literal(device_enum));
    }
    riscv::plic::irq target_irq = device_info[device_enum].irq;
    if (target_irq != riscv::plic::irq::NONE) {
      kernel::ExternController::Instance()->Register(target_irq, device);
    }
  }
}

driver::BasicDevice* DeviceFactory::GetDevice(DeviceList device) {
  if (device == DeviceList::disk0) {
    return &blk_device0_;    
  }
  if (device == DeviceList::uart0) {
    return &uart_;
  }
  return nullptr;
}

}  // namespace driver