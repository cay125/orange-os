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
}

void hartinit(int hart_id) {
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_ENABLE(hart_id), 1 << lib::common::literal(irq::VIRTIO0_IRQ));
  MEMORY_MAPPED_IO_W_WORD(memory_layout::PLIC_PRIORITY_THRESH(hart_id), 0);
  riscv::regs::mie.set_bit(riscv::MIE::MEIE);
}

}  // namespace plic
}  // namespace riscv
