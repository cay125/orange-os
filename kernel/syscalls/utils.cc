#include "kernel/syscalls/utils.h"

#include "arch/riscv_reg.h"
#include "kernel/regs_frame.hpp"
#include "kernel/scheduler.h"
#include "kernel/utils.h"

namespace kernel {
namespace syscall {
namespace comm {

uint64_t GetRawArg(int order) {
  if (order > 6 || order < 0) {
    panic("Invalid arg order: %d", order);
  }
  auto frame = Schedueler::Instance()->ThisProcess()->frame;
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
  }
  return -1;
}

void SetRawArg(int order, uint64_t value) {
  if (order > 6 || order < 0) {
    panic("Invalid arg order: %d", order);
  }
  auto frame = Schedueler::Instance()->ThisProcess()->frame;
  switch (order) {
  case 0:
    frame->a0 = value;
    return;
  case 1:
    frame->a1 = value;
    return;
  case 2:
    frame->a2 = value;
    return;
  case 3:
    frame->a3 = value;
    return;
  case 4:
    frame->a4 = value;
    return;
  case 5:
    frame->a5 = value;
    return;
  case 6:
    frame->a6 = value;
    return;
  }
}

int GetIntArg(int order) {
  return static_cast<int>(GetRawArg(order));
}

void SetIntArg(int order, int value) {
  SetRawArg(order, static_cast<uint64_t>(value));
}

}  // namespace comm
}  // namespace syscall
}  // naemspace kernel