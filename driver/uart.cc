#include "driver/uart.h"
#include "kernel/config/memory_layout.h"

namespace driver {

void put_char(char c) {
  MEMORY_MAPPED_IO_W_Byte(memory_layout::UART0, c);
}

}  // namespace driver