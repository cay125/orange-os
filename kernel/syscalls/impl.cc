#include <algorithm>
#include <span>

#include "arch/riscv_reg.h"
#include "driver/uart.h"
#include "kernel/config/memory_layout.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/utils.h"
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

}  // namespace syscall
}  // namespace kernel