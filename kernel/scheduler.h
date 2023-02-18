#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/config/system_param.h"
#include "kernel/cpu.h"
#include "kernel/process.h"
#include "lib/types.h"

namespace kernel {

struct SchedulerInfo {
  uint64_t mtime_cmp_addr;
  uint64_t interval;
};

class Schedueler {
 public:
  static Schedueler* Instance();
  void Yield();
  void Sleep();
  void ClockInterrupt();
  inline __attribute__((always_inline)) uint64_t SystemTick() {
    return ticks;
  }
  ProcessTask* ThisProcess();
  CpuTask* ThisCpu();
  void InitTimer();
  void Dispatch();
  ProcessTask* AllocProc();
  const SchedulerInfo* scheduler_info();

 private:
  Schedueler() {}
  static Schedueler sche_;

  ProcessTask process_tasks_[system_param::MAX_PROCESS_NUM];
  CpuTask cpu_task_[system_param::CPU_NUM];
  SchedulerInfo scheduler_info_;
  volatile uint64_t ticks = 0;
};

}  // namespace kernel

#endif  // KERNEL_SCHEDULER_H