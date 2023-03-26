#ifndef KERNEL_UTILS_STREAMBASE_H
#define KERNEL_UTILS_STREAMBASE_H

#include "lib/types.h"

namespace lib {

class StreamBase {
 public:
  StreamBase();
  StreamBase(const StreamBase&) = delete;
  StreamBase& operator= (const StreamBase&) = delete;

  virtual void Seek(size_t pos) = 0;
  virtual size_t Read(char* buf, size_t size) = 0;
  virtual size_t Size() const = 0;

 protected:
  size_t position_ = 0;
};

}  // namespace lib

#endif  // KERNEL_UTILS_STREAMBASE_H