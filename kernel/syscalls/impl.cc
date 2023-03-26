#include <algorithm>
#include <span>

#include "arch/riscv_reg.h"
#include "driver/uart.h"
#include "driver/device_factory.h"
#include "filesystem/common.h"
#include "filesystem/inode_def.h"
#include "filesystem/filestream.h"
#include "kernel/config/memory_layout.h"
#include "kernel/global_channel.h"
#include "kernel/scheduler.h"
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
  uint64_t* dst_page_table = VirtualMemory::Instance()->AllocProcessPageTable(dst_pro);
  if (!CopyMemory(src_pro->page_table, dst_page_table, src_pro->used_address, src_pro->used_address_size)) {
    return -1;
  }
  std::copy(src_pro->used_address.begin(), src_pro->used_address.begin() + src_pro->used_address_size, dst_pro->used_address.begin());
  dst_pro->used_address_size = src_pro->used_address_size;

  memcpy(dst_pro->user_sp, src_pro->user_sp, memory_layout::PGSIZE);
  dst_pro->saved_context.ra = reinterpret_cast<uint64_t>(ExecuteRet);

  // prepare trapframe
  memcpy(dst_pro->frame, src_pro->frame, sizeof(*dst_pro->frame));
  dst_pro->frame->mepc += 4;
  dst_pro->frame->a0 = 0;

  dst_pro->state = ProcessState::runnable;
  dst_pro->parent_process = src_pro;

  return 1;
}

int sys_write() {
  int fd = comm::GetIntArg(0);
  auto str = comm::GetAddrArg<char>(1);
  int size = comm::GetIntArg(2);
  auto root_pate = Schedueler::Instance()->ThisProcess()->page_table;
  auto str_pa = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_pate, (uint64_t)str));
  if (fd == 1) {
    for (int i = 0; i < size; ++i) 
      driver::put_char(str_pa[i]);
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
  if (fd->file_type == fs::FileType::disk_file) {
    fs::FileStream file_stream(fd->inode);
    file_stream.Seek(fd->offset);
    size = file_stream.Read(buf, size);
    fd->offset += size;
  } else if (fd->file_type == fs::FileType::none) {
    return -1;
  }
  return size;
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

int IteratorDir(fs::InodeDef* inode,
                driver::virtio::Device* device,
                const char* target_name) {
  uint32_t current_offset = 0;
  while (current_offset < inode->size) {
    uint32_t current_addr_index = current_offset / fs::BLOCK_SIZE;
    if (!inode->addr[current_addr_index]) {
      kernel::panic();
    }
    driver::virtio::MetaData meta_data;
    meta_data.block_index = inode->addr[current_addr_index];
    device->Operate(driver::virtio::Operation::read, &meta_data);
    auto p = meta_data.buf.begin();
    while (current_offset < inode->size && (p != meta_data.buf.end())) {
      auto* dir = reinterpret_cast<fs::DirElement*>(p);
      if (strcmp(target_name, dir->name) == 0) {
        return dir->inum;
      }
      p += fs::DIR_ELEMENT_SIZE;
      current_offset += fs::DIR_ELEMENT_SIZE;
    }
  }
  return -1;
}

int sys_open() {
  auto root_page = Schedueler::Instance()->ThisProcess()->page_table;
  auto path_name = reinterpret_cast<const char*>(VirtualMemory::Instance()->VAToPA(root_page, comm::GetRawArg(0)));
  if (!path_name || path_name[0] != '/') {
    return -1;
  }
  int path_level = -1;
  const char* paths[16] = {nullptr};
  const char* p = path_name;
  auto path_len = strlen(path_name);
  for (size_t i = 0; i < path_len;) {
    while (*p && *p != '/') {
      ++p;
    }
    if (*p && *(p + 1)) {
      ++p;
      paths[++path_level] = p;
    }
    i = p - path_name;
  }

  int current_inode_index = fs::ROOT_INODE;
  auto device = driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0);
  for (int i = 0; i <= path_level; ++i) {
    fs::InodeDef inode{};
    fs::GetInode(current_inode_index, &inode);
    if (i != path_level && inode.type != fs::inode_type::directory) {
      return -1;
    }
    int inum = IteratorDir(&inode, device, paths[i]);
    if (inum < 0) {
      return -1;
    }
    current_inode_index = inum;
  }

  auto& fds = Schedueler::Instance()->ThisProcess()->file_descriptor;
  for (auto it = fds.begin(); it != fds.end(); ++it) {
    if (it->file_type == fs::FileType::none) {
      it->file_type = fs::FileType::disk_file;
      it->inode_index = current_inode_index;
      fs::GetInode(it->inode_index, &it->inode);
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
  return 0;
}

}  // namespace syscall
}  // namespace kernel