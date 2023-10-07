#ifndef ARCH_RISCV_PLIC_H
#define ARCH_RISCV_PLIC_H

#include "lib/types.h"

namespace riscv {
namespace plic {

constexpr uint8_t MAX_PRIORITY = 7;
constexpr uint8_t MIN_PRIORITY = 0;

enum class irq : uint8_t {
  NONE = 0,
  VIRTIO0_IRQ = 1,
  VIRTIO1_IRQ = 2,
  VIRTIO2_IRQ = 3,
  VIRTIO3_IRQ = 4,
  UARRT0_IRQ = 10,
};

void globalinit();

void hartinit(int hart_id);

}  // plic
}  // namespace riscv

#endif  // ARCH_RISCV_PLIC_H