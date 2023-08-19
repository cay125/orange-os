#include "kernel/scheduler.h"

#include <tuple>

#include "arch/riscv_reg.h"
#include "kernel/config/memory_layout.h"
#include "kernel/config/system_param.h"
#include "kernel/global_channel.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/utils.h"
#include "kernel/printf.h"
#include "kernel/virtual_memory.h"

extern "C" void Switch(kernel::SavedContext* c1, kernel::SavedContext* c2);

uint64_t time_scratch[2];

namespace kernel {

void Schedueler::Yield() {
  CpuTask* current_cpu = &cpu_task_[riscv::regs::mhartid.read()];
  if (!current_cpu->process_task) {
    panic("process_task in empty when try to yield");
  }
  CriticalGuard guard(&current_cpu->process_task->lock);
  current_cpu->process_task->state = ProcessState::runnable;
  Switch(&current_cpu->process_task->saved_context,
           &current_cpu->saved_context);
}

void Schedueler::Exit(int code) {
  auto* process = ThisProcess();
  CriticalGuard guard(&process->lock);
  process->exit_code = code;
  process->state = ProcessState::zombie;
  for (auto& task : process_tasks_) {
    if (task.parent_process == process) {
      auto* init_process = GetInitProcess();
      if (!init_process) {
        kernel::panic("Cannot find init process");
      }
      task.parent_process = init_process;
    }
  }
  Wakeup(&process->parent_process->owned_channel);
  Switch(&process->saved_context,
         &ThisCpu()->saved_context);
}

void Schedueler::SetInitProcess(const ProcessTask* process) {
  init_process_ = process;
}
const ProcessTask* Schedueler::GetInitProcess() {
  return init_process_;
}

void Schedueler::InitTimer() {
  uint64_t hart_id = riscv::regs::mhartid.read();
  uint64_t mtime_cmp = memory_layout::CLINT_MTIMECMP(hart_id);
  uint64_t current_mtime = MEMORY_MAPPED_IO_R_DWORD(memory_layout::CLINT_MTIME);
  MEMORY_MAPPED_IO_W_DWORD(mtime_cmp, current_mtime + system_param::TIMER_INTERVAL);
  time_scratch[0] = mtime_cmp;
  time_scratch[1] = system_param::TIMER_INTERVAL;
  riscv::regs::mie.set_bit(riscv::MIE::MTIE);
}

void Schedueler::Sleep(const Channel* channel, SpinLock* lk) {
  CpuTask* current_cpu = &cpu_task_[riscv::regs::mhartid.read()];
  if (!current_cpu->process_task ||
      current_cpu->process_task->state != ProcessState::running) {
    panic("process_task's state is not running when try to sleep");
  }

  {
    CriticalGuard guard(&current_cpu->process_task->lock);
    if (lk) {
      lk->UnLock();
    }
    current_cpu->process_task->state = ProcessState::sleep;
    current_cpu->process_task->channel = channel;
    Switch(&current_cpu->process_task->saved_context,
           &current_cpu->saved_context);
    if (lk) {
      lk->Lock();
    }
  }
}

void Schedueler::Wakeup(const Channel* channel) {
  for (auto& task : process_tasks_) {
    CriticalGuard guard(&task.lock);
    if (task.state == ProcessState::sleep && (*task.channel) == (*channel)) {
      task.state = ProcessState::runnable;
      task.channel = nullptr;
    }
  }
}

std::tuple<bool, ProcessTask*> Schedueler::FindFirslZombieChild(const ProcessTask* parent){
  bool has_child = false;
  for (auto& task : process_tasks_) {
    if (task.parent_process == parent) {
      has_child = true;
      if (task.state == ProcessState::zombie) {
        return {true, &task};
      }
    }
  }
  return {has_child, nullptr};
}

void Schedueler::ClockInterrupt() {
  ticks = ticks + 1;
  Wakeup(GlobalChannel::sleep_channel());
}

ProcessTask* Schedueler::AllocProc() {
  for (ProcessTask& task : process_tasks_) {
    CriticalGuard guard(&task.lock);
    if (task.state == ProcessState::unused) {
      if (!task.Init()) {
        return nullptr;
      }
      task.state = ProcessState::used;
      return &task;
    }
  }
  return nullptr;
}

ProcessTask* Schedueler::ThisProcess() {
  CpuTask* current_cpu = &cpu_task_[riscv::regs::mhartid.read()];
  return current_cpu->process_task; 
}

CpuTask* Schedueler::ThisCpu() {
  return &cpu_task_[riscv::regs::mhartid.read()];
}

__attribute__ ((noreturn))
void Schedueler::Dispatch() {
  while (true) {
    global_interrunpt_on();
    CpuTask* current_cpu = &cpu_task_[riscv::regs::mhartid.read()];
    for (ProcessTask& task : process_tasks_) {
      CriticalGuard lk(&task.lock);
      if (task.state == ProcessState::runnable) {
        current_cpu->process_task = &task;
        task.state = ProcessState::running;
        Switch(&current_cpu->saved_context, &task.saved_context);
        current_cpu->process_task = nullptr;
      }
    }
  }
}

}  // namespace kernel