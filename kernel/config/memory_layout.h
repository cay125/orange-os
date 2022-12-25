#include "lib/types.h"

#ifndef KERNEL_CONFIG_MEMORY_LAYOUT_
#define KERNEL_CONFIG_MEMORY_LAYOUT_

#define MEMORY_MAPPED_IO_W_Byte(reg, value) (*(volatile uint8_t*)(reg) = value)
#define MEMORY_MAPPED_IO_R_Byte(reg) (*(volatile uint8_t*)(reg))

namespace memory_layout {

constexpr uint64_t UART0 = 0x10000000L;
constexpr uint64_t CLINT = 0x2000000L;
constexpr uint64_t CLINT_MTIME = 0x200BFF8;

constexpr uint64_t CLINT_MTIMECMP(int hart_id) {
  return 0x2004000L + 8 * (hart_id);
}

}  // namespace memory_layout

#endif // KERNEL_CONFIG_MEMORY_LAYOUT_