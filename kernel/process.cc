#include <algorithm>

#include "lib/string.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
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
  process->page_table = nullptr;
  process->frame = nullptr;
  process->kernel_sp = nullptr;
  process->user_sp = nullptr;
  process->used_address_size = 0;
  std::fill(process->used_address.begin(), process->used_address.end(), MemoryRegion(0, 0));
  process->state = ProcessState::unused;
  memset(&process->saved_context, 0, sizeof(process->saved_context));
  process->parent_process = nullptr;
  std::fill(process->current_path, process->current_path + strlen(process->current_path), 0);
  process->current_path[0] = '/';
  process->channel = nullptr;
  for (size_t i = 0; i < process->file_descriptor.size(); i++) {
    process->file_descriptor[i].file_type = fs::FileType::none;
  }
  process->pid = 0;
}

void ExecuteRet();

bool ProcessTask::Init(bool need_init_kernel_info) {
  auto* mem_manager = VirtualMemory::Instance();
  auto frame_page = mem_manager->Alloc();
  if (!frame_page) {
    return false;
  }
  auto root_page_table = mem_manager->Alloc();
  if (!root_page_table) {
    mem_manager->FreePage({frame_page});
    return false;
  }
  auto user_stack_page = mem_manager->Alloc();
  if (!user_stack_page) {
    mem_manager->FreePage({frame_page, root_page_table});
    return false;
  }
  uint64_t user_stack_page_va = VirtualMemory::GetUserSpVa() - memory_layout::PGSIZE;
  if (!mem_manager->MapPage(root_page_table, user_stack_page_va, (uint64_t)user_stack_page, riscv::PTE::R | riscv::PTE::W | riscv::PTE::U)) {
     mem_manager->FreePage({frame_page, root_page_table, user_stack_page});
    return false;
  }
  if (need_init_kernel_info) {
    auto sp_page = mem_manager->Alloc();
    if (!sp_page) {
      frame = nullptr;
      mem_manager->FreePage({frame_page, root_page_table, user_stack_page});
      return false;
    }
    kernel_sp = sp_page;
    pid = ProcessManager::AllocPid();
    saved_context.sp = reinterpret_cast<uint64_t>(kernel_sp) + memory_layout::PGSIZE;
    saved_context.ra = reinterpret_cast<uint64_t>(ExecuteRet);
  }
  page_table = root_page_table;
  frame = reinterpret_cast<RegFrame*>(frame_page);
  user_sp = user_stack_page;
  frame->kernel_sp = (uint64_t)kernel_sp + memory_layout::PGSIZE;
  frame->sp = user_stack_page_va + memory_layout::PGSIZE;
  frame->scheduler_info = Schedueler::Instance()->scheduler_info();
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
