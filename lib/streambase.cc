#include "lib/streambase.h"

namespace lib {

StreamBase::StreamBase() {}

char& StreamBase::operator[] (size_t index) const {
  return At(index);
}

StreamBase& StreamBase::operator+= (size_t index) {
  Seek(position_ + index);
  return *this;
}

}  // namespace lib