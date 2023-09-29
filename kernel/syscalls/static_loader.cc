#include "kernel/process.h"
#include "kernel/syscalls/static_loader.h"
#include "kernel/printf.h"
#include "kernel/virtual_memory.h"
#include <elf.h>

namespace kernel {

StaticLoader::StaticLoader(lib::StreamBase* stream, ProcessTask* process, uint64_t offset) : stream_(stream), process_(process), offset_(offset) {

}

static bool LoadSegment(uint64_t* root_page, lib::StreamBase* stream, uint64_t va, size_t size) {
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

bool StaticLoader::Load(uint8_t target_type, uint64_t* entry, DynamicInfo* dynamic_info) {
  Elf64_Ehdr elf64_header;
  stream_->Seek(0);
  stream_->ReadForType(&elf64_header);
  auto& elf_ident = elf64_header.e_ident;
  // check magic number
  if (elf_ident[EI_MAG0] != ELFMAG0 || elf_ident[EI_MAG1] != ELFMAG1 || elf_ident[EI_MAG2] != ELFMAG2 || elf_ident[EI_MAG3] != ELFMAG3) {
    printf("exec: not a valid elf64 file\n");
    printf("%x %c %c %c\n", elf_ident[0],elf_ident[1],elf_ident[2],elf_ident[3]);
    return false;
  }
  if (elf_ident[EI_CLASS] != ELFCLASS64) {
    printf("exec: only support 64bit program\n");
    return false;
  }
  if (elf_ident[EI_DATA] != ELFDATA2LSB) {
    printf("exec: only support little endian\n");
    return false;
  }
  if (elf_ident[EI_VERSION] != EV_CURRENT) {
    printf("exec: elf version must be 1\n");
    return false;
  }
  if (elf64_header.e_type != target_type) {
    printf("exec: not a exec program, type: %d\n", elf64_header.e_type);
    return false;
  }
  if (elf64_header.e_machine != EM_RISCV) {
    printf("exec: not a valid riscv program\n");
    return false;
  }

  uint64_t* root_page = process_->page_table;

  for (int i = 0; i < elf64_header.e_phnum; ++i) {
    Elf64_Phdr ph;
    stream_->Seek(elf64_header.e_phoff + i * elf64_header.e_phentsize);
    stream_->ReadForType(&ph);
    if (ph.p_type == PT_DYNAMIC) {
      DynamicInfo info{ph.p_vaddr + offset_, ph.p_memsz};
      if (target_type == ET_EXEC) {
        process_->dynamic_info = info;
      }
      if (dynamic_info) {
        *dynamic_info = info;
      }
    }
    if (ph.p_type != PT_LOAD) {
      continue;
    }
    if (ph.p_memsz < ph.p_filesz) {
      printf("exec: p_memsz should bigger than p_filesz\n");
      return false;
    }
    riscv::PTE privi = riscv::PTE::None;
    if (ph.p_flags & PF_R) {
      privi |= riscv::PTE::R;
    }
    if (ph.p_flags & PF_W) {
      privi |= riscv::PTE::W;
    }
    if (ph.p_flags & PF_X) {
      privi |= riscv::PTE::X;
    }
    uint64_t va_beg = ph.p_vaddr + offset_;
    VirtualMemory::Instance()->MapMemory(root_page, va_beg, va_beg + ph.p_memsz, privi | riscv::PTE::U);
    stream_->Seek(ph.p_offset);
    if (!LoadSegment(root_page, stream_, va_beg, ph.p_filesz)) {
      printf("exec: Load Segment failed\n");
      return false;
    }
    if (process_->used_address_size >= process_->used_address.size()) {
      printf("exec: too many load-segment\n");
      return false;
    }
    auto& region = process_->used_address[process_->used_address_size];
    region.first = va_beg;
    region.second = va_beg + ph.p_memsz;
    process_->used_address_size += 1;
  }
  if (entry) {
    *entry = elf64_header.e_entry;
  }
  return true;
}

}