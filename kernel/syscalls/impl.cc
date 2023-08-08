#include <algorithm>
#include <span>

#include "arch/riscv_reg.h"
#include "driver/uart.h"
#include "driver/device_factory.h"
#include "filesystem/common.h"
#include "filesystem/inode_def.h"
#include "filesystem/filestream.h"
#include "filesystem/file_descriptor.h"
#include "kernel/config/memory_layout.h"
#include "kernel/global_channel.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/resource_factory.h"
#include "kernel/scheduler.h"
#include "kernel/return_code/syscall_err.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/utils.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/string.h"

namespace kernel {
namespace syscall {

static bool CopyMemory(uint64_t* src_root_page,
                       uint64_t* dst_root_page,
                       const std::span<MemoryRegion>& region,
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

int sys_fork() {
  auto* src_pro = Schedueler::Instance()->ThisProcess();
  auto* dst_pro = Schedueler::Instance()->AllocProc();
  if (!dst_pro) {
    return -1;
  }
  if (!CopyMemory(src_pro->page_table, dst_pro->page_table, src_pro->used_address, src_pro->used_address_size)) {
    return -1;
  }
  std::copy(src_pro->used_address.begin(), src_pro->used_address.begin() + src_pro->used_address_size, dst_pro->used_address.begin());
  dst_pro->used_address_size = src_pro->used_address_size;

  memcpy(dst_pro->user_sp, src_pro->user_sp, memory_layout::PGSIZE);

  // prepare trapframe
  memcpy(dst_pro->frame, src_pro->frame, sizeof(*dst_pro->frame));
  dst_pro->frame->kernel_sp = reinterpret_cast<uint64_t>(dst_pro->kernel_sp) + memory_layout::PGSIZE;
  dst_pro->frame->a0 = 0;

  dst_pro->state = ProcessState::runnable;
  dst_pro->parent_process = src_pro;

  return dst_pro->pid;
}

int sys_write() {
  int fd = comm::GetIntArg(0);
  auto str = comm::GetAddrArg<char>(1);
  int size = comm::GetIntArg(2);
  auto root_pate = Schedueler::Instance()->ThisProcess()->page_table;
  auto str_pa = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_pate, (uint64_t)str));
  if (fd == 1) {
    for (int i = 0; i < size; ++i) 
      driver::Uart::put_char(str_pa[i]);
  }
  return 0;
}

static void GetFileDescriptor(int fd_index, fs::FileDescriptor** taregt_fd) {
  auto* cur_process = Schedueler::Instance()->ThisProcess();
  if (fd_index < 0 || (uint64_t)fd_index >= cur_process->file_descriptor.size()) {
    return;
  }
  auto& fd = cur_process->file_descriptor[fd_index];
  if (fd.file_type == fs::FileType::none) {
    return;
  }
  *taregt_fd = &fd;
}

int sys_read() {
  fs::FileDescriptor* fd = nullptr;
  GetFileDescriptor(comm::GetIntArg(0), &fd);
  if (!fd) {
    return -1;
  }
  auto* root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto buf = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(1)));
  if (!buf) {
    return -1;
  }
  auto size = comm::GetIntegralArg<size_t>(2);
  if (fd->file_type == fs::FileType::regular_file || fd->file_type == fs::FileType::directory) {
    fs::FileStream file_stream(fd->inode);
    file_stream.Seek(fd->offset);
    size = file_stream.Read(buf, size);
    fd->offset += size;
  } else if (fd->file_type == fs::FileType::device) {
    auto* r = ResourceFactory::Instance()->GetResource(fd->inode.major, fd->inode.minor);
    if (!r) {
      return -1;
    }
    size = r->read(buf, size);
  } else if (fd->file_type == fs::FileType::none) {
    return -1;
  }
  return size;
}

int sys_exec() {
  ProcessTask* process = Schedueler::Instance()->ThisProcess();
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(0)));
  char** argv = reinterpret_cast<char**>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(1)));
  if (!argv) {
    return -1;
  }
  fs::InodeDef inode{};
  auto inode_index = fs::Open(path_name, &inode);
  if (inode_index < 0) {
    return -1;
  }
  auto* new_page = VirtualMemory::Instance()->Alloc();
  if (!new_page) {
    return -1;
  }
  if (sizeof(ProcessTask) > memory_layout::PGSIZE) {
    kernel::panic("page size cannot cantain ProcessTask");
  }
  ProcessTask* tmp_process_task = reinterpret_cast<ProcessTask*>(new_page);
  tmp_process_task->kernel_sp = process->kernel_sp;
  if (!tmp_process_task->Init(false)) {
    return -1;
  }
  CriticalGuard guard;
  fs::FileStream filestream(inode);
  int ret = ExecuteImpl(&filestream, tmp_process_task);
  if (ret < 0) {
    tmp_process_task->FreePageTable(false);
    return -1;
  }
  auto user_sp_begin = reinterpret_cast<uint64_t>(tmp_process_task->user_sp) + memory_layout::PGSIZE;
  uint64_t user_sp = user_sp_begin;
  uint64_t argv_addr[17];
  int argc = 0;
  for (argc = 0; ; argc++) {
    if (!argv[argc]) {
      user_sp -= (argc + 1) * 8;
      user_sp -= user_sp % 16;
      argv_addr[argc] = 0;
      break;
    }
    const char* str = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(process->page_table, reinterpret_cast<uint64_t>(argv[argc])));
    user_sp -= strlen(str) + 1;
    user_sp -= user_sp % 16;
    argv_addr[argc] = tmp_process_task->frame->sp - (user_sp_begin - user_sp);
    std::copy(str, str + strlen(str) + 1, reinterpret_cast<char*>(user_sp));
  }
  std::copy(argv_addr, argv_addr + argc + 1, reinterpret_cast<uint64_t*>(user_sp));
  tmp_process_task->frame->sp -= user_sp_begin - user_sp;
  tmp_process_task->frame->a1 = tmp_process_task->frame->sp;
  process->FreePageTable(false);
  process->CopyMemoryFrom(tmp_process_task);
  VirtualMemory::Instance()->FreePage(new_page);
  return argc;
}

int sys_sleep() {
  uint64_t current_tick = Schedueler::Instance()->SystemTick();
  auto sleep_time = comm::GetIntegralArg<uint64_t>(0);
  while ((Schedueler::Instance()->SystemTick() - current_tick) < sleep_time) {
    Schedueler::Instance()->Sleep(GlobalChannel::sleep_channel());
  }
  if ((Schedueler::Instance()->SystemTick() - current_tick) >= sleep_time) {
    return 0;
  } else {
    return sleep_time - (Schedueler::Instance()->SystemTick() - current_tick);
  }
}

int sys_open() {
  auto root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(0)));
  fs::InodeDef inode{};
  auto inode_index = fs::Open(path_name, &inode);
  if (inode_index < 0) {
    return -1;
  }
  auto& fds = Schedueler::Instance()->ThisProcess()->file_descriptor;
  for (auto it = fds.begin(); it != fds.end(); ++it) {
    if (it->file_type == fs::FileType::none) {
      it->file_type = inode.type;
      it->inode_index = inode_index;
      it->inode = inode;
      it->offset = 0;
      return it - fds.begin();
    }
  }
  return -1;
}

int sys_fstat() {
  fs::FileDescriptor* fd = nullptr;
  GetFileDescriptor(comm::GetIntArg(0), &fd);
  if (!fd) {
    return -1;
  }
  auto* root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto f_stat = reinterpret_cast<fs::FileState*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(1)));
  f_stat->inode_index = fd->inode_index;
  f_stat->link_count = fd->inode.link_count;
  f_stat->size = fd->inode.size;
  f_stat->type = fd->file_type;
  return 0;
}

int sys_mknod() {
  auto root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(0)));
  uint8_t major = comm::GetIntArg(1);
  uint8_t minor = comm::GetIntArg(2);
  bool ret = fs::Create(path_name, fs::FileType::device, major, minor);
  if (!ret) {
    return -1;
  }
  return 0;
}

int sys_getpid() {
  return Schedueler::Instance()->ThisProcess()->pid;
}

int sys_exit() {
  Schedueler::Instance()->Exit();
  return 0;
}

int sys_wait() {
  auto* process = Schedueler::Instance()->ThisProcess();
  while (true) {
    auto* child = Schedueler::Instance()->FindFirslChild(process);
    if (!child) {
      return -1;
    }
    CriticalGuard guard(&child->lock);
    if (child->state == ProcessState::zombie) {
      int pid = child->pid;
      ProcessManager::ResetProcess(child);
      return pid;
    }
    Schedueler::Instance()->Sleep(&child->owned_channel, &child->lock);
  }
  return 0;
}

int sys_getcwd() {
  auto* process = Schedueler::Instance()->ThisProcess();
  auto addr = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(0)));
  auto max_len = comm::GetIntegralArg<size_t>(1);
  size_t path_len = strlen(process->current_path);
  if (path_len > max_len) {
    return -1;
  }
  std::copy(process->current_path, process->current_path + path_len, addr);
  addr[path_len] = '\0';
  return 0;
}

int sys_chdir() {
  auto* process = Schedueler::Instance()->ThisProcess();
  auto* path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(0)));
  fs::InodeDef inode{};
  auto inode_index = fs::Open(path_name, &inode);
  if (inode_index < 0) {
    return return_code::no_such_file_or_directory;
  }
  if (inode.type != fs::FileType::directory) {
    return return_code::not_a_directory;
  }
  auto path_len = strlen(path_name);
  if (path_name[0] != '/') {
    std::copy(path_name, path_name + path_len + 1, process->current_path + strlen(process->current_path));
  } else {
    std::copy(path_name, path_name + path_len + 1, process->current_path);
  }
  return 0;
}

}  // namespace syscall
}  // namespace kernel