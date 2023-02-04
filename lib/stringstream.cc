#include <algorithm>

#include "lib/stringstream.h"

namespace lib {

StringStream::StringStream(char* s, size_t size) : StreamBase() {
  buf_ = s;
  size_ = size;
}

void StringStream::Seek(size_t pos) {
  position_ = std::min(pos, size_ - 1);
}

size_t StringStream::Read(char* buf, size_t size) {
  size = std::min(size_ - position_, size);
  std::copy(buf_ + position_, buf_ + position_ + size, buf);
  return size;
}

size_t StringStream::Size() const {
  return size_;
}

char& StringStream::At(size_t index) const {
  return *(buf_ + position_ + index);
}

char* StringStream::Head() const {
  return buf_ + position_;
}

char* StringStream::operator+ (size_t index) const {
  return buf_ + position_ + index;
}

}  // namespace lib