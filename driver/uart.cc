#include "driver/uart.h"
#include "kernel/config/memory_layout.h"
#include "lib/common.h"

namespace driver {

enum class UartReg : uint8_t {
  RHR = 0,
  IER = 1,
  FCR = 2,
  LSR = 5,
};

#define IER_RX_ENABLE (1<<0)
#define IER_TX_ENABLE (1<<1)
#define FCR_FIFO_ENABLE (1<<0)
#define FCR_FIFO_CLEAR (3<<1)
using lib::common::literal;

bool Uart::Init(uint64_t addr) {
  // disable interrupts
  MEMORY_MAPPED_IO_W_Byte(addr + literal(UartReg::IER), 0x00);
  // reset and enable FIFOs
  MEMORY_MAPPED_IO_W_Byte(addr + literal(UartReg::FCR), FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
  // enable receive interrupts
  MEMORY_MAPPED_IO_W_Byte(addr + literal(UartReg::IER), IER_RX_ENABLE);

  addr_ = addr;
  is_writable_ = true;
  return true;
}

void Uart::Write(const char* buf, size_t size) {
  for (size_t i = 0; i < size; i++) {
    put_char(buf[i]);
  }
}

void Uart::ProcessInterrupt() {
  while (true) {
    if(MEMORY_MAPPED_IO_R_Byte(addr_ + literal(UartReg::LSR)) & 0x01) {
      // input data is ready.
      uint8_t c = MEMORY_MAPPED_IO_R_Byte(addr_ + literal(UartReg::RHR));
      if (interrupt_callback) {
        char buf[1] = {c};
        interrupt_callback(buf, 1);
      }
    } else {
      break;
    }
  }
}

void Uart::put_char(char c) {
  MEMORY_MAPPED_IO_W_Byte(memory_layout::UART0, c);
}

}  // namespace driver