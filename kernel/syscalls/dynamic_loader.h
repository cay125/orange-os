#pragma once

#include <elf.h>

#include "kernel/process.h"
#include "kernel/syscalls/static_loader.h"
#include "lib/streambase.h"

namespace kernel {

class DynamicLoader : public StaticLoader {
 public:
  DynamicLoader(lib::StreamBase* stream, ProcessTask* process, uint64_t offset);
  bool LoadDynamicInfo(const DynamicInfo* dynamic_info, uint64_t* init_array_beg, uint64_t* init_array_end);

 private:
  std::array<uint64_t, 2> ProcessDynamicInfo(uint64_t offset, const Elf64_Shdr* symtab, const Elf64_Shdr* symstrtab, const DynamicInfo* dynamic_info, bool need_iterate);
};

}
