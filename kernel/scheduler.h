#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/config/system_param.h"
#include "kernel/cpu.h"
#include "kernel/process.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace kernel {

class SpinLock;

class Schedueler : public lib::Singleton<Schedueler> {
 public:
  friend class lib::Singleton<Schedueler>;
  void Yield();
  void Sleep(const Channel* channel, SpinLock* lk = nullptr);
  void Wakeup(const Channel* channel);
  void Exit(int code);
  void SetInitProcess(const ProcessTask* process);
  const ProcessTask* GetInitProcess();
  std::tuple<bool, ProcessTask*> FindFirslZombieChild(const ProcessTask* parent);
  void ClockInterrupt();
  inline __attribute__((always_inline)) uint64_t SystemTick() {
    return ticks;
  }
  ProcessTask* ThisProcess();
  CpuTask* ThisCpu();
  void InitTimer();
  void Dispatch();
  ProcessTask* AllocProc();

 private:
  Schedueler() {}

  ProcessTask process_tasks_[system_param::MAX_PROCESS_NUM];
  CpuTask cpu_task_[system_param::CPU_NUM];
  volatile uint64_t ticks = 0;
  const ProcessTask* init_process_;
};

}  // namespace kernel

#endif  // KERNEL_SCHEDULER_H