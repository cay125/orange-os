#include <algorithm>

#include "arch/riscv_isa.h"
#include "arch/riscv_reg.h"
#include "kernel/config/memory_layout.h"
#include "kernel/printf.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/define.h"
#include "kernel/syscalls/elf_def.h"
#include "kernel/utils.h"
#include "kernel/virtual_memory.h"
#include "lib/types.h"
#include "lib/string.h"
#include "lib/streambase.h"
#include "lib/stringstream.h"

extern "C" {
void restore_user_context(kernel::RegFrame* frame);
void exception_table();
}

namespace kernel {

bool LoadSegment(uint64_t* root_page, char* p, uint64_t va, size_t size) {
  while (size != 0) {
    uint64_t pa = VirtualMemory::Instance()->VAToPA(root_page, va);
    if (pa == 0) {
      return false;
    }
    uint64_t remain_size = memory_layout::PGSIZE - pa % memory_layout::PGSIZE;
    size_t len = size > remain_size ? remain_size : size;
    size -= len;
    std::copy(p, p + len, reinterpret_cast<uint8_t*>(pa));
    p += len;
    va = VirtualMemory::AddrCastDown(va) + memory_layout::PGSIZE;
  }
  return true;
}

int ExecuteImpl(lib::StreamBase* stream, ProcessTask* process) {
  auto& p = *stream;
  if (p[EI_MAG0] != ELFMAG0 || p[EI_MAG1] != ELFMAG1 || p[EI_MAG2] != ELFMAG2 || p[EI_MAG3] != ELFMAG3) {
    printf("exec: not a valid elf64 file\n");
    printf("%x %c %c %c\n", p[0],p[1],p[2],p[3]);
    return -1;
  }
  if (p[EI_CLASS] != ELFCLASS64) {
    printf("exec: only support 64bit program\n");
    return -1;
  }
  if (p[EI_DATA] != ELFDATA2LSB) {
    printf("exec: only support little endian\n");
    return -1;
  }
  if (p[EI_VERSION] != EV_CURRENT) {
    printf("exec: elf version must be 1\n");
    return -1;
  }
  const elf::Elf64_Ehdr* elf64_header = reinterpret_cast<const elf::Elf64_Ehdr*>(p.Head());
  if (elf64_header->e_type != ET_EXEC) {
    printf("exec: not a exec program\n");
    return -1;
  }
  if (elf64_header->e_machine != EM_RISCV) {
    printf("exec: not a valid riscv program\n");
    return -1;
  }

  uint64_t* root_page = kernel::VirtualMemory::Instance()->AllocProcessPageTable(process);

  for (int i = 0; i < elf64_header->e_phnum; ++i) {
    const char* ph_head_p = p + elf64_header->e_phoff + i * elf64_header->e_phentsize;
    const elf::Elf64_Phdr* ph = reinterpret_cast<const elf::Elf64_Phdr*>(ph_head_p);
    if (ph->p_type != PT_LOAD) {
      continue;
    }
    if (ph->p_memsz < ph->p_filesz) {
      printf("exec: p_memsz should bigger than p_filesz\n");
      return -1;
    }
    riscv::PTE privi = riscv::PTE::None;
    if (ph->p_flags & PF_R) {
      privi |= riscv::PTE::R;
    }
    if (ph->p_flags & PF_W) {
      privi |= riscv::PTE::W;
    }
    if (ph->p_flags & PF_X) {
      privi |= riscv::PTE::X;
    }
    VirtualMemory::Instance()->MapMemory(root_page, ph->p_vaddr, ph->p_vaddr + ph->p_memsz, privi | riscv::PTE::U);
    if (!LoadSegment(root_page, p + ph->p_offset, ph->p_vaddr, ph->p_filesz)) {
      printf("exec: Load Segment failed\n");
      return -1;
    }
    if (process->used_address_size >= process->used_address.size()) {
      printf("exec: too many load-segment\n");
      return -1;
    }
    auto& region = process->used_address[process->used_address_size];
    region.first = ph->p_vaddr;
    region.second = ph->p_vaddr + ph->p_memsz;
    process->used_address_size += 1;
  }
  process->frame->mepc = elf64_header->e_entry;

  return 0;
}

void TrapRet(ProcessTask* process, riscv::Exception exception) {
  global_interrunpt_off();
  RegFrame* frame = process->frame;
  if (exception != riscv::Exception::environment_call_from_u_mode) {
    riscv::regs::satp.write(riscv::virtual_addresing::Sv39, process->page_table);
    riscv::isa::sfence();
  }
  riscv::regs::mepc.write(frame->mepc);
  riscv::regs::mtvec.write(reinterpret_cast<uint64_t>(exception_table) + 1);
  riscv::regs::mstatus.set_mpp(riscv::MPP::user_mode);
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mpie);
  restore_user_context(frame);
}

void ExecuteRet() {
  TrapRet(Schedueler::Instance()->ThisProcess(), riscv::Exception::none);
}

void ExcuteInitProcess(char* code) {
  lib::StringStream stringstream(code, strlen(code));
  ProcessTask* process = Schedueler::Instance()->AllocProc();
  ExecuteImpl(&stringstream, process);
  process->state = ProcessState::runnable;
  process->saved_context.ra = reinterpret_cast<uint64_t>(ExecuteRet);
}

}  // namespace kernel