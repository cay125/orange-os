#ifndef KERNEL_CONFIG_MEMORY_LAYOUT_
#define KERNEL_CONFIG_MEMORY_LAYOUT_

#include "lib/types.h"

#define MEMORY_MAPPED_IO_W_Byte(reg, value) (*(volatile uint8_t*)(reg) = value)
#define MEMORY_MAPPED_IO_R_Byte(reg) (*(volatile uint8_t*)(reg))

#define MEMORY_MAPPED_IO_W_WORD(reg, value) (*(volatile uint32_t*)(reg) = value)
#define MEMORY_MAPPED_IO_R_WORD(reg) (*(volatile uint32_t*)(reg))

#define MEMORY_MAPPED_IO_W_DWORD(reg, value) (*(volatile uint64_t*)(reg) = value)
#define MEMORY_MAPPED_IO_R_DWORD(reg) (*(volatile uint64_t*)(reg))

namespace memory_layout {

constexpr uint64_t UART0 = 0x10000000L;
constexpr uint64_t CLINT = 0x2000000L;
constexpr uint64_t CLINT_MTIME = 0x200BFF8;

constexpr uint64_t PLIC_BASE = 0x0c000000uL;
constexpr uint64_t PLIC_PRIORITY = PLIC_BASE + 0x0;
constexpr uint64_t PLIC_PENDING = PLIC_BASE + 0x1000;

constexpr uint64_t PLIC_ENABLE(int hart_id) {
  return PLIC_BASE + 0x2000 + hart_id * 0x100;
}

constexpr uint64_t PLIC_PRIORITY_THRESH(int hart_id) {
  return PLIC_BASE + 0x200000 + hart_id * 0x2000;
}

constexpr uint64_t PLIC_CLAIM(int hart_id) {
  return PLIC_BASE + 0x200004 + hart_id * 0x2000;
}

constexpr uint64_t CLINT_MTIMECMP(int hart_id) {
  return 0x2004000L + 8 * (hart_id);
}

constexpr uint64_t VIRTIO0 = 0x10001000;
constexpr uint64_t VIRTIO1 = 0x10002000;
constexpr uint64_t VIRTIO2 = 0x10003000;
constexpr uint64_t VIRTIO3 = 0x10004000;

constexpr uint64_t KERNEL_BASE = 0x8000'0000L;
constexpr uint64_t MEMORY_END = KERNEL_BASE + 128 * 1024 * 1024;
constexpr uint64_t MAX_SUPPORT_PA = 0x3f'ffff'ffff'ffffL;
constexpr uint64_t MAX_SUPPORT_VA = 0x3f'ffff'ffffL;
constexpr uint64_t PGSIZE = 4096;

}  // namespace memory_layout

#endif // KERNEL_CONFIG_MEMORY_LAYOUT_