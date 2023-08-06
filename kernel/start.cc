#include "arch/riscv_isa.h"
#include "arch/riscv_plic.h"
#include "arch/riscv_reg.h"
#include "driver/device_factory.h"
#include "lib/types.h"
#include "kernel/config/memory_layout.h"
#include "kernel/config/system_param.h"
#include "kernel/console.h"
#include "kernel/printf.h"
#include "kernel/resource_factory.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"

__attribute__ ((aligned (memory_layout::PGSIZE))) char stack[system_param::CPU_NUM][2 * memory_layout::PGSIZE];

extern void (*init_array_start[])(void);
extern void (*init_array_end[])(void);

namespace kernel {

void RunInitArray() {
  int count = init_array_end - init_array_start;
  for (int i = 0; i < count; ++i) {
    uint64_t addr = reinterpret_cast<uint64_t>(init_array_start[i]);
    if (addr == 0 || addr == 0xffff'ffff'ffff'ffff) {
      continue;
    }
    init_array_start[i]();
  }
}

}  // namespace kernel

extern char initcode[];
extern long initcode_size;

extern "C" {
void kernel_exception_table();
}

void start() {
  kernel::RunInitArray();
  riscv::regs::tp.write(riscv::regs::mhartid.read());
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mie);
  riscv::regs::mtvec.write_vec(kernel_exception_table, true);

  if (!kernel::VirtualMemory::Instance()->Init()) {
    kernel::panic();
  }

  riscv::regs::pmp_addr.write(0, 0x3f'ffff'ffff'ffffuL);
  riscv::regs::pmp_cfg.write(0, riscv::PMPBit::X | riscv::PMPBit::R | riscv::PMPBit::W | riscv::PMPBit::TOR);

  riscv::plic::globalinit();
  riscv::plic::hartinit(riscv::regs::mhartid.read());
  driver::DeviceFactory::Instance()->InitDevices();

  kernel::Console console(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::uart0));
  kernel::ResourceFactory::Instance()->RegistResource(kernel::resource_id::console_major, kernel::resource_id::console_minor, &console);

  kernel::ExcuteInitProcess(initcode, initcode_size);
  kernel::Schedueler::Instance()->InitTimer();
  kernel::Schedueler::Instance()->Dispatch();
}
