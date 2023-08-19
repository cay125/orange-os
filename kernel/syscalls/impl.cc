#include <algorithm>

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
#include "kernel/sys_def/syscall_err.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/utils.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/string.h"

namespace kernel {
namespace syscall {

int sys_fork() {
  auto* src_pro = Schedueler::Instance()->ThisProcess();
  auto* dst_pro = Schedueler::Instance()->AllocProc();
  if (!dst_pro) {
    return -1;
  }
  if (!dst_pro->CopyFrom(src_pro)) {
    return -1;
  }
  dst_pro->frame->a0 = 0;
  dst_pro->state = ProcessState::runnable;
  dst_pro->parent_process = src_pro;
  return dst_pro->pid;
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

int sys_write() {
  fs::FileDescriptor* fd = nullptr;
  GetFileDescriptor(comm::GetIntArg(0), &fd);
  if (!fd) {
    printf("sys_write: Invalid file_descriptor: %d\n", comm::GetIntArg(0));
    return -1;
  }
  auto* root_page = Schedueler::Instance()->ThisProcess()->page_table;
  riscv::PTE pte = riscv::PTE::None;
  auto buf = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(1), &pte));
  if (!buf || !(pte & riscv::PTE::U) || (pte & riscv::PTE::X)) {
    printf("sys_write: Trying to write from invalid memory\n");
    return -1;
  }
  auto size = comm::GetIntegralArg<size_t>(2);
  if (fd->file_type == fs::FileType::regular_file || fd->file_type == fs::FileType::directory) {
    fs::FileStream file_stream(fd->inode);
    file_stream.Seek(fd->offset);
    size = file_stream.write(buf, size);
    fd->offset += size;
  } else if (fd->file_type == fs::FileType::device) {
    auto* r = ResourceFactory::Instance()->GetResource(fd->inode.major, fd->inode.minor);
    if (!r) {
      return -1;
    }
    size = r->write(buf, size);
  } else if (fd->file_type == fs::FileType::none) {
    printf("sys_write: Invalid file_descriptor: %d\n", comm::GetIntArg(0));
    return -1;
  }
  return size;
}

int sys_read() {
  fs::FileDescriptor* fd = nullptr;
  GetFileDescriptor(comm::GetIntArg(0), &fd);
  if (!fd) {
    printf("sys_read: Invalid file_descriptor: %d\n", comm::GetIntArg(0));
    return -1;
  }
  auto* root_page = Schedueler::Instance()->ThisProcess()->page_table;
  riscv::PTE pte = riscv::PTE::None;
  auto buf = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(1), &pte));
  if (!buf || !(pte & riscv::PTE::U) || (pte & riscv::PTE::X)) {
    printf("sys_read: Trying to read data to invalid memory\n");
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
    printf("sys_read: Invalid file_descriptor: %d\n", comm::GetIntArg(0));
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
  uint64_t addr = comm::GetRawArg(1);
  if (!addr) {
    return -1;
  }
  auto* root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto f_stat = reinterpret_cast<fs::FileState*>(VirtualMemory::Instance()->VAToPA(root_page, addr));
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
  int exit_code = comm::GetIntArg(0);
  Schedueler::Instance()->Exit(exit_code);
  return 0;
}

int sys_wait() {
  auto* process = Schedueler::Instance()->ThisProcess();
  while (true) {
    bool has_child;
    ProcessTask* child;
    std::tie(has_child, child) = Schedueler::Instance()->FindFirslZombieChild(process);
    if (!has_child) {
      return -1;
    }
    if (child) {
      CriticalGuard guard(&child->lock);
      int pid = child->pid;
      if (comm::GetRawArg(0)) {
        auto addr = reinterpret_cast<int*>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(0)));
        *addr = child->exit_code;
      }
      ProcessManager::ResetProcess(child);
      return pid;
    }
    Schedueler::Instance()->Sleep(&process->owned_channel);
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
  const char* path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(process->page_table, comm::GetRawArg(0)));
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
    auto cur_path_len = strlen(process->current_path);
    if (process->current_path[cur_path_len - 1] != '/') {
      process->current_path[cur_path_len] = '/';
      process->current_path[cur_path_len + 1] = '\0';
    }
    std::copy(path_name, path_name + path_len + 1, process->current_path + strlen(process->current_path));
  } else {
    std::copy(path_name, path_name + path_len + 1, process->current_path);
  }
  // remove last '/'
  auto cur_path_len = strlen(process->current_path);
  if (cur_path_len > 1 && process->current_path[cur_path_len - 1] == '/') {
    process->current_path[cur_path_len - 1] = '\0';
  }
  return 0;
}

int sys_mkdir() {
  auto root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(0)));
  bool ret = fs::Create(path_name, fs::FileType::directory, 0, 0);
  if (!ret) {
    return -1;
  }
  return 0;
}

int sys_sbrk() {
  auto* process = Schedueler::Instance()->ThisProcess();
  uint32_t bytes = comm::GetIntegralArg<uint32_t>(0);
  if (bytes == 0) {
    return 0;
  }
  uint64_t max_mem_addr = 0;
  for (size_t i = 0; i < process->used_address_size; i++) {
    if (process->used_address[i].second > max_mem_addr) {
      max_mem_addr = process->used_address[i].second;
    }
  }
  max_mem_addr = VirtualMemory::AddrCastUp(max_mem_addr);
  bool ret = VirtualMemory::Instance()->MapMemory(process->page_table, max_mem_addr, max_mem_addr + bytes, riscv::PTE::R | riscv::PTE::W | riscv::PTE::U);
  if (!ret) {
    return 0;
  }
  process->used_address[process->used_address_size] = {max_mem_addr, max_mem_addr + bytes};
  process->used_address_size += 1;
  return max_mem_addr;
}

}  // namespace syscall
}  // namespace kernel