#ifndef KERNEL_UTILS_STRINGSTREAM_H
#define KERNEL_UTILS_STRINGSTREAM_H

#include "lib/streambase.h"

namespace lib {

class StringStream : public StreamBase {
 public:
  StringStream(char* s, size_t size);
  StringStream(const StringStream&) = delete;
  StringStream& operator= (const StringStream&) = delete;
  
  virtual void Seek(size_t pos) override;
  virtual size_t Read(char* buf, size_t size) override;
  virtual size_t Size() const override;
  virtual char& At(size_t index) const override;
  virtual char* Head() const override;

  virtual char* operator+ (size_t index) const override;

 private:
  char* buf_ = nullptr;
  size_t size_ = 0;
};

}  // namespace lib

#endif  // KERNEL_UTILS_STRINGSTREAM_H