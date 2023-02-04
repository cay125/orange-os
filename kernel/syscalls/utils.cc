#include "kernel/syscalls/utils.h"

#include "arch/riscv_reg.h"
#include "kernel/regs_frame.hpp"
#include "kernel/utils.h"

namespace kernel {
namespace syscall {
namespace comm {

uint64_t GetRawArg(int order) {
  if (order > 6 || order < 0) {
    panic();
  }
  auto frame = reinterpret_cast<RegFrame*>(riscv::regs::mscratch.read());
  switch (order) {
  case 0:
    return frame->a0;
  case 1:
    return frame->a1;
  case 2:
    return frame->a2;
  case 3:
    return frame->a3;
  case 4:
    return frame->a4;
  case 5:
    return frame->a5;
  case 6:
    return frame->a6;
  default:
    panic();
  }
  return -1;
}

int GetIntArg(int order) {
  return static_cast<int>(GetRawArg(order));
}

}  // namespace comm
}  // namespace syscall
}  // naemspace kernel