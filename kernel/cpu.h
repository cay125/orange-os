#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include "kernel/process.h"

namespace kernel {

struct CpuTask {
  // current process run on this cpu
  ProcessTask* process_task = nullptr;
  SavedContext saved_context;
};

}  // namespace kernel

#endif  // KERNEL_CPU_H