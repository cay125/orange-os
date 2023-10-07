#include <algorithm>
#include <assert.h>
#include <span>

#include "lib/string.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/printf.h"
#include "kernel/process.h"
#include "kernel/regs_frame.hpp"
#include "kernel/scheduler.h"
#include "kernel/sys_def/descriptor_def.h"
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
  const char* root_path = "/";
  std::copy(root_path, root_path + strlen(root_path) + 1, process->current_path);
  process->channel = nullptr;
  for (fs::FileDescriptor& fd : process->file_descriptor) {
    memset(&fd, 0, sizeof(fd));
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
  assert(sizeof(RegFrame) == 2048);
  user_sp = user_stack_page;
  frame->kernel_sp = (uint64_t)kernel_sp + memory_layout::PGSIZE;
  frame->sp = user_stack_page_va + memory_layout::PGSIZE;
  if (file_descriptor[descriptor_def::io::console].file_type != fs::FileType::none) {
    printf("Error: fd in descriptor_def::io::console is not empty\n");
    return false;
  }
  file_descriptor[descriptor_def::io::console].file_type = fs::FileType::device;
  file_descriptor[descriptor_def::io::console].inode.major = 0;
  file_descriptor[descriptor_def::io::console].inode.minor = 0;
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
  dynamic_info = process->dynamic_info;
}

static bool CopyMemory(uint64_t* src_root_page,
                       uint64_t* dst_root_page,
                       const std::span<const MemoryRegion>& region,
                       size_t used_address_size) {
  for (size_t i = 0; i < used_address_size; ++i) {
    riscv::PTE pte = riscv::PTE::None;
    uint64_t va_beg = region[i].first, va_end = region[i].second;
    auto size = va_end - va_beg;
    while (size > 0) {
      uint64_t src_pa = VirtualMemory::Instance()->VAToPA(src_root_page, va_beg, &pte);
      uint64_t remain_size = memory_layout::PGSIZE - src_pa % memory_layout::PGSIZE;
      size_t len = size > remain_size ? remain_size : size;
      if (!VirtualMemory::Instance()->MapMemory(dst_root_page, va_beg, va_beg + len, pte)) {
        return false;
      }
      uint64_t dst_pa = VirtualMemory::Instance()->VAToPA(dst_root_page, va_beg);
      memcpy(reinterpret_cast<uint8_t*>(dst_pa), reinterpret_cast<uint8_t*>(src_pa), len);
      size -= len;
      va_beg = VirtualMemory::AddrCastDown(va_beg) + memory_layout::PGSIZE;
    }
  }
  return true;
}

bool ProcessTask::CopyFrom(const ProcessTask* src_process) {
  if (!CopyMemory(src_process->page_table, page_table, src_process->used_address, src_process->used_address_size)) {
    return false;
  }
  std::copy(src_process->used_address.begin(), src_process->used_address.begin() + src_process->used_address_size, used_address.begin());
  used_address_size = src_process->used_address_size;

  memcpy(user_sp, src_process->user_sp, memory_layout::PGSIZE);

  // prepare trapframe
  memcpy(frame, src_process->frame, sizeof(*frame));
  frame->kernel_sp = reinterpret_cast<uint64_t>(kernel_sp) + memory_layout::PGSIZE;
  memcpy(current_path, src_process->current_path, strlen(src_process->current_path));
  dynamic_info = src_process->dynamic_info;
  return true;
}

uint64_t ProcessTask::ExpandMemory(size_t bytes) {
  if (bytes == 0) {
    return 0;
  }
  uint64_t max_mem_addr = 0;
  for (size_t i = 0; i < used_address_size; i++) {
    if (used_address[i].second > max_mem_addr) {
      max_mem_addr = used_address[i].second;
    }
  }
  max_mem_addr = VirtualMemory::AddrCastUp(max_mem_addr);
  if (!VirtualMemory::Instance()->MapMemory(page_table, max_mem_addr, max_mem_addr + bytes, riscv::PTE::R | riscv::PTE::W | riscv::PTE::U)) {
    return 0;
  }
  used_address[used_address_size] = {max_mem_addr, max_mem_addr + bytes};
  used_address_size += 1;
  return max_mem_addr;
}

}  // namespace kernel
