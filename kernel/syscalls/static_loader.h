#pragma once

#include "kernel/process.h"
#include "lib/streambase.h"

namespace kernel {

class StaticLoader {
 public:
  StaticLoader(lib::StreamBase* stream, ProcessTask* process, uint64_t offset);
  
  virtual bool Load(uint8_t target_type, uint64_t* entry, DynamicInfo* dynamic_info);

 protected:
  lib::StreamBase* stream_;
  ProcessTask* process_;
  uint64_t offset_;
};

}