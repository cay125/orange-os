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
void user_exception_table();
}

namespace kernel {

bool LoadSegment(uint64_t* root_page, lib::StreamBase* stream, uint64_t va, size_t size) {
  auto batch_load = [&stream](size_t len, char* dst){
    const int memory_batch = 512;
    while (len > 0) {
      size_t remain = len > memory_batch ? memory_batch : len;
      char p[memory_batch] = {0};
      stream->Read(p, remain);
      std::copy(p, p + remain, dst);
      len -= remain;
      dst += remain;
    }
  };
  while (size != 0) {
    uint64_t pa = VirtualMemory::Instance()->VAToPA(root_page, va);
    if (pa == 0) {
      return false;
    }
    uint64_t remain_size = memory_layout::PGSIZE - pa % memory_layout::PGSIZE;
    size_t len = size > remain_size ? remain_size : size;
    size -= len;
    batch_load(len, reinterpret_cast<char*>(pa));
    va = VirtualMemory::AddrCastDown(va) + memory_layout::PGSIZE;
  }
  return true;
}

int ExecuteImpl(lib::StreamBase* stream, ProcessTask* process) {
  char elf_ident[EI_NIDENT] = {0};
  stream->Read(elf_ident, sizeof(elf_ident));
  if (elf_ident[EI_MAG0] != ELFMAG0 || elf_ident[EI_MAG1] != ELFMAG1 || elf_ident[EI_MAG2] != ELFMAG2 || elf_ident[EI_MAG3] != ELFMAG3) {
    printf("exec: not a valid elf64 file\n");
    printf("%x %c %c %c\n", elf_ident[0],elf_ident[1],elf_ident[2],elf_ident[3]);
    return -1;
  }
  if (elf_ident[EI_CLASS] != ELFCLASS64) {
    printf("exec: only support 64bit program\n");
    return -1;
  }
  if (elf_ident[EI_DATA] != ELFDATA2LSB) {
    printf("exec: only support little endian\n");
    return -1;
  }
  if (elf_ident[EI_VERSION] != EV_CURRENT) {
    printf("exec: elf version must be 1\n");
    return -1;
  }
  stream->Seek(0);
  char elf_header[sizeof(elf::Elf64_Ehdr)] = {0};
  stream->Read(elf_header, sizeof(elf_header));
  const elf::Elf64_Ehdr* elf64_header = reinterpret_cast<const elf::Elf64_Ehdr*>(elf_header);
  if (elf64_header->e_type != ET_EXEC) {
    printf("exec: not a exec program, type: %d\n", elf64_header->e_type);
    return -1;
  }
  if (elf64_header->e_machine != EM_RISCV) {
    printf("exec: not a valid riscv program\n");
    return -1;
  }

  uint64_t* root_page = kernel::VirtualMemory::Instance()->AllocProcessPageTable(process);

  for (int i = 0; i < elf64_header->e_phnum; ++i) {
    stream->Seek(elf64_header->e_phoff + i * elf64_header->e_phentsize);
    char ph_head_p[sizeof(elf::Elf64_Phdr)] = {0};
    stream->Read(ph_head_p, sizeof(ph_head_p));
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
    stream->Seek(ph->p_offset);
    if (!LoadSegment(root_page, stream, ph->p_vaddr, ph->p_filesz)) {
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
  riscv::regs::satp.write(riscv::virtual_addresing::Sv39, process->page_table);
  riscv::isa::sfence();
  riscv::regs::mepc.write(frame->mepc);
  riscv::regs::mtvec.write_vec(user_exception_table, true);
  riscv::regs::mstatus.set_mpp(riscv::MPP::user_mode);
  riscv::regs::mstatus.clear_bit(riscv::StatusBit::mpie);
  restore_user_context(frame);
}

void ExecuteRet() {
  auto* process = Schedueler::Instance()->ThisProcess();
  process->lock.UnLock();
  TrapRet(process, riscv::Exception::none);
}

void ExcuteInitProcess(char* code, size_t size) {
  lib::StringStream stringstream(code, size);
  ProcessTask* process = Schedueler::Instance()->AllocProc();
  ExecuteImpl(&stringstream, process);
  process->state = ProcessState::runnable;
}

}  // namespace kernel