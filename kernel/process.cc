#include <algorithm>

#include "lib/string.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/process.h"
#include "kernel/virtual_memory.h"

namespace kernel {

int cur_pid = 0;
SpinLock pid_lock;

int ProcessManager::AllocPid() {
  CriticalGuard guard(&pid_lock);
  return ++cur_pid;
}

void ProcessManager::ResetProcess(ProcessTask* process) {
  process->FreePageTable();
  process->state = ProcessState::unused;
  process->parent_process = nullptr;
  std::fill(process->current_path, process->current_path + strlen(process->current_path), 0);
  process->current_path[0] = '/';
  process->channel = nullptr;
}

void ExecuteRet();

bool ProcessTask::Init(bool need_init_kernel_info) {
  auto frame_page = VirtualMemory::Instance()->Alloc();
  if (!frame_page) {
    return false;
  }
  frame = reinterpret_cast<RegFrame*>(frame_page);
  auto root_page_table = VirtualMemory::Instance()->Alloc();
  if (!root_page_table) {
    VirtualMemory::Instance()->FreePage({frame_page});
    return false;
  }
  if (need_init_kernel_info) {
    auto sp_page = VirtualMemory::Instance()->Alloc();
    if (!sp_page) {
      frame = nullptr;
      VirtualMemory::Instance()->FreePage({frame_page, root_page_table});
      return false;
    }
    kernel_sp = sp_page;
    pid = ProcessManager::AllocPid();
    saved_context.sp = reinterpret_cast<uint64_t>(kernel_sp) + memory_layout::PGSIZE;
    saved_context.ra = reinterpret_cast<uint64_t>(ExecuteRet);
  }
  page_table = root_page_table;
  return true;
}

void ProcessTask::FreePageTable(bool need_free_kernel_page) {
  for (int level = 3; level > 0; level--) {
    for (size_t i = 0; i < used_address_size; i++) {
      VirtualMemory::Instance()->FreeMemory(page_table, used_address[i].first, used_address[i].second, level);
    }
    VirtualMemory::Instance()->FreeMemory(page_table, VirtualMemory::Instance()->GetUserSpVa() - memory_layout::PGSIZE, VirtualMemory::Instance()->GetUserSpVa(), level);
  }
  VirtualMemory::Instance()->FreePage(reinterpret_cast<uint64_t*>(frame));
  VirtualMemory::Instance()->FreePage(page_table);
  if (need_free_kernel_page) {
    VirtualMemory::Instance()->FreePage(kernel_sp);
  }
}

void ProcessTask::CopyMemoryFrom(const ProcessTask* process) {
  page_table = process->page_table;
  frame = process->frame;
  used_address_size = process->used_address_size;
  used_address = process->used_address;
  user_sp = process->user_sp;
}

}  // namespace kernel
