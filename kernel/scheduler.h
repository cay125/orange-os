#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/config/system_param.h"
#include "kernel/cpu.h"
#include "kernel/process.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace kernel {

class SpinLock;

struct SchedulerInfo {
  uint64_t mtime_cmp_addr;
  uint64_t interval;
};

class Schedueler : public lib::Singleton<Schedueler> {
 public:
  friend class lib::Singleton<Schedueler>;
  void Yield();
  void Sleep(Channel* channel, SpinLock* lk = nullptr);
  void Wakeup(Channel* channel);
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

  ProcessTask process_tasks_[system_param::MAX_PROCESS_NUM];
  CpuTask cpu_task_[system_param::CPU_NUM];
  SchedulerInfo scheduler_info_;
  volatile uint64_t ticks = 0;
};

}  // namespace kernel

#endif  // KERNEL_SCHEDULER_H