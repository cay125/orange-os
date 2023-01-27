#include "arch/riscv_reg.h"
#include "driver/uart.h"
#include "kernel/config/memory_layout.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/utils.h"
#include "kernel/virtual_memory.h"

namespace kernel {
namespace syscall {

int sys_fork() {
  return 0;
}

int sys_write() {
  int fd = comm::GetIntArg(0);
  auto str = comm::GetAddrArg<char>(1);
  int size = comm::GetIntArg(2);
  auto root_pate = reinterpret_cast<uint64_t*>((riscv::regs::satp.read() & 0xfff'ffff'ffffL) * memory_layout::PGSIZE);
  auto str_pa = reinterpret_cast<char*>(VirtualMemory::Instance()->VAToPA(root_pate, (uint64_t)str));
  if (fd == 1) {
    for (int i = 0; i < size; ++i) 
      driver::put_char(str_pa[i]);
  }
  return 0;
}

}  // namespace syscall
}  // namespace kernel