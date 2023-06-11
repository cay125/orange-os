#ifndef DRIVER_UART_H_
#define DRIVER_UART_H_

#include "driver/basic_device.h"
namespace driver {

class Uart : public BasicDevice {
 public:
  static void put_char(char c);
  bool Init(uint64_t addr) override;
  void ProcessInterrupt() override;

 private:
  uint64_t addr_;
  void(*interrupt_callback)(char) = nullptr;
};

}  // namespace driver

#endif  // DRIVER_UART_H_
