#include <elf.h>

#include <array>
#include <type_traits>
#include <utility>

#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include "kernel/syscalls/dynamic_loader.h"
#include "kernel/virtual_memory.h"
#include "lib/string.h"

namespace kernel {

DynamicLoader::DynamicLoader(lib::StreamBase* stream, ProcessTask* process, uint64_t offset) : StaticLoader(stream, process, offset) {

}

static uint64_t iterate_symtab(const Elf64_Shdr* symtab, const Elf64_Shdr* symstr_tab, const char* target_sym_name, lib::StreamBase* stream) {
  auto sym_num = symtab->sh_size / sizeof(Elf64_Sym);
  for (uint32_t j = 0; j < sym_num; j++) {
    stream->Seek(symtab->sh_offset + j * sizeof(Elf64_Sym));
    Elf64_Sym sym;
    stream->ReadForType(&sym);
    stream->Seek(symstr_tab->sh_offset + sym.st_name);
    char buf[64];
    stream->GetLine(buf);
    if (strcmp(buf, target_sym_name) == 0) {
      return sym.st_value;
    }
  }
  return 0;
}

template<class T, class F>
std::pair<const char*, uint64_t> process_sym(uint64_t offset, const Elf64_Dyn* relo_dyn, const Elf64_Dyn* sym_hdr, const Elf64_Dyn* symstr_hdr, F f, uint32_t index) {
  auto* sym_dyn = reinterpret_cast<T*>(f(relo_dyn->d_un.d_ptr + offset + index * sizeof(T)));
  auto* sym = reinterpret_cast<Elf64_Sym*>(f(sym_hdr->d_un.d_ptr + offset + ELF64_R_SYM(sym_dyn->r_info) * sizeof(Elf64_Sym)));
  auto* sym_name = reinterpret_cast<const char*>(f(sym->st_name + symstr_hdr->d_un.d_ptr + offset));
  auto relo_type = ELF64_R_TYPE(sym_dyn->r_info);
  if (relo_type == R_RISCV_64) {
    *reinterpret_cast<uint64_t*>(f(sym_dyn->r_offset + offset)) = sym->st_value + offset;
  } else if (relo_type == R_RISCV_JUMP_SLOT) {
    *reinterpret_cast<uint64_t*>(f(sym_dyn->r_offset + offset)) = sym->st_value + offset;
  } else if (relo_type == R_RISCV_RELATIVE) {
    if constexpr (std::is_same_v<T, Elf64_Rela>) {
      *reinterpret_cast<uint64_t*>(f(sym_dyn->r_offset + offset)) = sym_dyn->r_addend + offset;
    }
  }
  return {sym_name, sym_dyn->r_offset + offset};
}

std::array<uint64_t, 2> DynamicLoader::ProcessDynamicInfo(uint64_t offset, const Elf64_Shdr* symtab, const Elf64_Shdr* symstrtab, const DynamicInfo* dynamic_info, bool need_iterate) {
  Elf64_Dyn* rela_hdr = nullptr;
  Elf64_Dyn* rela_size = nullptr;
  Elf64_Dyn* rel_hdr = nullptr;
  Elf64_Dyn* rel_size = nullptr;
  Elf64_Dyn* sym_hdr = nullptr;
  Elf64_Dyn* symstr_hdr = nullptr;
  Elf64_Dyn* init_array = nullptr;
  Elf64_Dyn* init_array_sz = nullptr;

  auto dyn_info_num = dynamic_info->dynamic_size / sizeof(Elf64_Dyn);
  for (uint32_t i = 0; i < dyn_info_num; ++i) {
    auto* dyn = reinterpret_cast<Elf64_Dyn*>(VirtualMemory::Instance()->VAToPA(process_->page_table, dynamic_info->dynamic_vaddr + i * sizeof(Elf64_Dyn)));
    if (dyn->d_tag == DT_RELA) {
      rela_hdr = dyn;
    }
    if (dyn->d_tag == DT_RELASZ) {
      rela_size = dyn;
    }
    if (dyn->d_tag == DT_REL) {
      rel_hdr = dyn;
    }
    if (dyn->d_tag == DT_RELSZ) {
      rel_size = dyn;
    }
    if (dyn->d_tag == DT_SYMTAB) {
      sym_hdr = dyn;
    }
    if (dyn->d_tag == DT_STRTAB) {
      symstr_hdr = dyn;
    }
    if (dyn->d_tag == DT_INIT_ARRAY) {
      init_array = dyn;
    }
    if (dyn->d_tag == DT_INIT_ARRAYSZ) {
      init_array_sz = dyn;
    }
  }
  if (!sym_hdr || !symstr_hdr) {
    printf("DynamicLoader: Cannot find dyn sym from memory\n");
    return {};
  }
  auto va_to_pa_util = [this](uint64_t va) -> uint64_t {
    return VirtualMemory::Instance()->VAToPA(process_->page_table, va);
  };
  auto fun = [&](Elf64_Dyn* hdr, Elf64_Dyn* size, auto type){
    if (!hdr || !size) {
      return;
    }
    auto need_relo_num = rela_size->d_un.d_val / (sizeof(decltype(type)));
    for (uint32_t i = 0; i < need_relo_num; ++i) {
      auto target = process_sym<decltype(type)>(offset, rela_hdr, sym_hdr, symstr_hdr, va_to_pa_util, i);
      if (need_iterate) {
        auto addr = iterate_symtab(symtab, symstrtab, target.first, stream_);
        if (addr > 0) {
          printf("Loading symbol: ['%s'], value: %#x, offset: %#x\n", target.first, addr, offset_);
          *reinterpret_cast<uint64_t*>(va_to_pa_util(target.second)) = offset_ + addr;
        } else {
          printf("Cannot find target symbol: ['%s']\n", target.first);
        }
      }
    }
  };
  fun(rela_hdr, rela_size, Elf64_Rela());
  fun(rel_hdr, rel_size, Elf64_Rel());
  if (init_array && init_array_sz) {
    return {init_array->d_un.d_ptr, init_array->d_un.d_ptr + init_array_sz->d_un.d_val};
  }
  return {};
}

bool DynamicLoader::LoadDynamicInfo(const DynamicInfo* dynamic_info, uint64_t* init_array_beg, uint64_t* init_array_end) {
  Elf64_Ehdr elf_header;
  stream_->Seek(0);
  stream_->ReadForType(&elf_header);
  if (elf_header.e_type != ET_DYN) {
    printf("DynamicLoader: Invalid elf type, need dyn while got: %d\n", elf_header.e_type);
    return false;
  }
  Elf64_Shdr shstr_header;
  stream_->Seek(elf_header.e_shoff + elf_header.e_shstrndx * elf_header.e_shentsize);
  stream_->ReadForType(&shstr_header);
  Elf64_Shdr dynstr_header;
  bool is_find_dynstr_header = false;
  Elf64_Shdr dynsym_header;
  bool is_find_dynsym_header = false;
  for (int i = 0; i < elf_header.e_shnum; ++i) {
    if (i == elf_header.e_shstrndx) {
      continue;
    }
    Elf64_Shdr section_header;
    stream_->Seek(elf_header.e_shoff + i * elf_header.e_shentsize);
    stream_->ReadForType(&section_header);
    stream_->Seek(shstr_header.sh_offset + section_header.sh_name);
    char buf[64];
    stream_->GetLine(buf);
    if (strcmp(buf, ".dynstr") == 0) {
      dynstr_header = section_header;
      is_find_dynstr_header = true;
    }
    if (strcmp(buf, ".dynsym") == 0) {
      dynsym_header = section_header;
      is_find_dynsym_header = true;
    }
    if (is_find_dynstr_header && is_find_dynsym_header) {
      break;
    }
  }
  if (!is_find_dynstr_header || !is_find_dynsym_header) {
    printf("DynamicLoader: Cannot find dyn sym section from .so\n");
    return false;
  }

  ProcessDynamicInfo(0, &dynsym_header, &dynstr_header, &process_->dynamic_info, true);
  auto init_array = ProcessDynamicInfo(offset_, &dynsym_header, &dynstr_header, dynamic_info, false);
  
  if (init_array_beg && init_array_end) {
    *init_array_beg = init_array[0] + offset_;
    *init_array_end = init_array[1] + offset_;
  }

  return true;
}

}