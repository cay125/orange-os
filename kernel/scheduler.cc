#include "kernel/scheduler.h"

#include "kernel/config/memory_layout.h"
#include "kernel/global_channel.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"

extern "C" void Switch(kernel::SavedContext* c1, kernel::SavedContext* c2);

namespace kernel {

const SchedulerInfo* Schedueler::scheduler_info() {
  return &scheduler_info_;
}

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

void Schedueler::InitTimer() {
  uint64_t mtime_cmp = memory_layout::CLINT_MTIMECMP(riscv::regs::mhartid.read());
  uint64_t current_mtime = MEMORY_MAPPED_IO_R_DWORD(memory_layout::CLINT_MTIME);
  MEMORY_MAPPED_IO_W_DWORD(mtime_cmp, current_mtime + system_param::TIMER_INTERVAL);
  scheduler_info_.mtime_cmp_addr = mtime_cmp;
  scheduler_info_.interval = system_param::TIMER_INTERVAL;
  riscv::regs::mie.set_bit(riscv::MIE::MTIE);
}

void Schedueler::Sleep(Channel* channel, SpinLock* lk) {
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

void Schedueler::Wakeup(Channel* channel) {
  for (auto& task : process_tasks_) {
    CriticalGuard guard(&task.lock);
    if (task.state == ProcessState::sleep && (*task.channel) == (*channel)) {
      task.state = ProcessState::runnable;
      task.channel = nullptr;
    }
  }
}

void Schedueler::ClockInterrupt() {
  ticks = ticks + 1;
  Wakeup(GlobalChannel::sleep_channel());
}

ProcessTask* Schedueler::AllocProc() {
  for (ProcessTask& task : process_tasks_) {
    if (task.state == ProcessState::unused) {
      auto frame_page = VirtualMemory::Instance()->Alloc();
      if (!frame_page) {
        return nullptr;
      }
      task.frame = reinterpret_cast<RegFrame*>(frame_page);
      auto sp_page = VirtualMemory::Instance()->Alloc();
      if (!sp_page) {
        task.frame = nullptr;
        VirtualMemory::Instance()->FreePage(frame_page);
        return nullptr;
      }
      auto root_page_table = VirtualMemory::Instance()->Alloc();
      if (!root_page_table) {
        VirtualMemory::Instance()->FreePage({frame_page, sp_page});
        return nullptr;
      }
      task.page_table = root_page_table;
      task.kernel_sp = sp_page;
      task.saved_context.sp = reinterpret_cast<uint64_t>(task.kernel_sp) + memory_layout::PGSIZE;
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