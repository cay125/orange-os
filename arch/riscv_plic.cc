#include "arch/riscv_plic.h"
#include "arch/riscv_reg.h"
#include "kernel/config/memory_layout.h"
#include "kernel/utils.h"
#include "lib/common.h"

namespace riscv {
namespace plic {

static void set_priority_hander(irq e, uint8_t priority) {
  if (priority > MAX_PRIORITY || priority < MIN_PRIORITY) {
    return;
  }
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_BASE + lib::common::literal(e) * 4, priority);
}

void globalinit() {
  // set interrupt irq priority
  set_priority_hander(irq::VIRTIO0_IRQ, 1);
  set_priority_hander(irq::VIRTIO1_IRQ, 1);
  set_priority_hander(irq::VIRTIO2_IRQ, 1);
  set_priority_hander(irq::VIRTIO3_IRQ, 1);
  set_priority_hander(irq::VIRTIO4_IRQ, 1);
  set_priority_hander(irq::UARRT0_IRQ, 1);
}

using lib::common::literal;

void hartinit(int hart_id) {
  uint32_t enabled_irq = 0;
  enabled_irq |= 1 << literal(irq::VIRTIO0_IRQ);
  enabled_irq |= 1 << literal(irq::VIRTIO1_IRQ);
  enabled_irq |= 1 << literal(irq::VIRTIO2_IRQ);
  enabled_irq |= 1 << literal(irq::VIRTIO3_IRQ);
  enabled_irq |= 1 << literal(irq::VIRTIO4_IRQ);
  enabled_irq |= 1 << literal(irq::UARRT0_IRQ);
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_ENABLE(hart_id), enabled_irq);
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_PRIORITY_THRESH(hart_id), 0);
  riscv::regs::mie.set_bit(riscv::MIE::MEIE);
}

}  // namespace plic
}  // namespace riscv
