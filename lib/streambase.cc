#include "lib/streambase.h"

namespace lib {

StreamBase::StreamBase() {}

void StreamBase::GetLine(char *buf) {
  do {
    Read(buf, 1);
  } while (*buf++ != '\0');
}

}  // namespace lib