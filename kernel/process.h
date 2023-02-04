#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include "kernel/regs_frame.hpp"
#include "lib/types.h"

namespace kernel {

struct SavedContext {
  uint64_t ra;
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
  uint64_t sp;
};

enum class ProcessState : uint8_t {
  unused,
  sleep,
  runnable,
  running,
};

struct ProcessTask {
  const char* name = nullptr;
  uint64_t pid = 0;
  RegFrame* frame = nullptr;
  uint64_t* page_table = nullptr;
  ProcessState state = ProcessState::unused;
  SavedContext saved_context;
};

}  // namespace kernel

#endif  // KERNEL_PROCESS_H