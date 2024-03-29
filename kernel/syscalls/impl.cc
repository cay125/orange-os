#include <algorithm>

#include "arch/riscv_reg.h"
#include "driver/uart.h"
#include "driver/device_factory.h"
#include "driver/virtio_gpu.h"
#include "filesystem/common.h"
#include "filesystem/inode_def.h"
#include "filesystem/filestream.h"
#include "filesystem/file_descriptor.h"
#include "kernel/config/memory_layout.h"
#include "kernel/global_channel.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/printf.h"
#include "kernel/process.h"
#include "kernel/resource_factory.h"
#include "kernel/scheduler.h"
#include "kernel/sys_def/device_info.h"
#include "kernel/sys_def/syscall_err.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/dynamic_loader.h"
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
  auto size = comm::GetIntegralArg<size_t>(2);
  if (fd->file_type == fs::FileType::regular_file || fd->file_type == fs::FileType::directory) {
    fs::FileStream file_stream(fd->inode);
    file_stream.Seek(fd->offset);
    auto fun = [&file_stream](const char* buf, size_t len) -> size_t {
      return file_stream.write(buf, len);
    };
    size = safe_copy(comm::GetRawArg(1), size, fun);
    fd->offset += size;
  } else if (fd->file_type == fs::FileType::device) {
    auto* r = ResourceFactory::Instance()->GetResource(fd->inode.major, fd->inode.minor);
    if (!r) {
      return -1;
    }
    auto fun = [r](const char* buf, size_t len) -> size_t {
      return r->write(buf, len);
    };
    size = safe_copy(comm::GetRawArg(1), size, fun);
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
  auto size = comm::GetIntegralArg<size_t>(2);
  if (fd->file_type == fs::FileType::regular_file || fd->file_type == fs::FileType::directory) {
    fs::FileStream file_stream(fd->inode);
    file_stream.Seek(fd->offset);
    auto fun = [&file_stream](char* buf, size_t len) -> size_t {
      return file_stream.Read(buf, len);
    };
    size = safe_copy(comm::GetRawArg(1), size, fun);
    fd->offset += size;
  } else if (fd->file_type == fs::FileType::device) {
    auto* r = ResourceFactory::Instance()->GetResource(fd->inode.major, fd->inode.minor);
    if (!r) {
      return -1;
    }
    auto fun = [r](char* buf, size_t len) -> size_t {
      return r->read(buf, len);
    };
    size = safe_copy(comm::GetRawArg(1), size, fun);
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

int sys_close() {
  fs::FileDescriptor* fd = nullptr;
  GetFileDescriptor(comm::GetIntArg(0), &fd);
  if (!fd) {
    return -1;
  }
  memset(fd, 0, sizeof(fs::FileDescriptor));
  return 0;
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
  return process->ExpandMemory(bytes);
}

int sys_uptime() {
  uint64_t current_tick = Schedueler::Instance()->SystemTick();
  return current_tick;
}

int sys_create() {
  auto root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(0)));
  fs::FileType file_type = static_cast<fs::FileType>(comm::GetIntArg(1));
  if (file_type != fs::FileType::directory && file_type != fs::FileType::regular_file) {
    return -1;
  }
  fs::InodeDef target_inode;
  bool ret = fs::Create(path_name, file_type, 0, 0, &target_inode);
  if (!ret) {
    return -1;
  }
  auto& fds = Schedueler::Instance()->ThisProcess()->file_descriptor;
  for (auto it = fds.begin(); it != fds.end(); ++it) {
    if (it->file_type == fs::FileType::none) {
      it->file_type = file_type;
      it->inode_index = target_inode.inode_index;
      it->inode = target_inode;
      it->offset = 0;
      return it - fds.begin();
    }
  }
  return -1;
}

int sys_get_screen_info() {
  auto* info = comm::GetAddrArg<device_info::screen_info>(0);
  if (!info) {
    printf("sys_get_screen_info: invalid addr: \n");
    return -1;
  }
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  driver::virtio::gpu::virtio_gpu_resp_display_info display_info = gpu_device->GetDisplayInfo();
  info->height = display_info.pmodes[0].r.height;
  info->width = display_info.pmodes[0].r.width;
  return 0;
}

int sys_framebuffer() {
  auto* process = Schedueler::Instance()->ThisProcess();
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  driver::virtio::gpu::virtio_gpu_resp_display_info info = gpu_device->GetDisplayInfo();
  auto& rect = info.pmodes[0].r;
  size_t memory_size = rect.height * rect.width * 4;
  uint64_t addr = process->ExpandMemory(memory_size);
  bool map_ret = gpu_device->SetupFramebuffer(rect, 0, addr, memory_size);
  if (!map_ret) {
    return 0;
  }
  return addr;
}

int sys_frame_flush() {
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  gpu_device->Flush();
  return 0;
}

int sys_setup_cursor() {
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  auto* cursor_image = reinterpret_cast<driver::virtio::gpu::CursorImage*>(comm::GetRawArg(0));
  gpu_device->SetupCursor(cursor_image, 0, 0, 0, 0);
  return 0;
}

int sys_move_cursor() {
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  gpu_device->MoveCursor(comm::GetIntegralArg<uint32_t>(0), comm::GetIntegralArg<uint32_t>(1));
  return 0;
}

int sys_detach_framebuffer() {
  auto* gpu_device = reinterpret_cast<driver::virtio::GPUDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::gpu0));
  bool ret = gpu_device->Drop();
  if (!ret) {
    return -1;
  }
  return 0;
}

int sys_dlopen() {
  ProcessTask* process = Schedueler::Instance()->ThisProcess();
  if (process->dynamic_info.dynamic_vaddr == 0 || process->dynamic_info.dynamic_size == 0) {
    printf("dlopen: No dynamic symbol needed\n");
    return 0;
  }
  char path_name[64];
  comm::GetStrArg(0, path_name, sizeof(path_name));
  fs::InodeDef inode{};
  auto inode_index = fs::Open(path_name, &inode);
  if (inode_index < 0) {
    printf("dlopen: Invalid path: %s\n", path_name);
    return -1;
  }
  printf("dlopen: Loading symbol from %s\n", path_name);
  CriticalGuard guard;
  fs::FileStream filestream(inode);
  uint64_t max_mem_addr = 0;
  for (size_t i = 0; i < process->used_address_size; i++) {
    if (process->used_address[i].second > max_mem_addr) {
      max_mem_addr = process->used_address[i].second;
    }
  }
  DynamicLoader dynamic_loader(&filestream, process, VirtualMemory::AddrCastUp(max_mem_addr));
  DynamicInfo dynamic_info;
  if (!dynamic_loader.Load(ET_DYN, nullptr, &dynamic_info)) {
    return -1;
  }
  uint64_t init_array_beg, init_array_end;
  if (!dynamic_loader.LoadDynamicInfo(&dynamic_info, &init_array_beg, &init_array_end)) {
    return -1;
  }
  process->frame->a1 = init_array_beg;
  process->frame->a2 = init_array_end;
  return 0;
}

int sys_dlclose() {
  return 0;
}

}  // namespace syscall
}  // namespace kernel