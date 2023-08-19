#include <algorithm>

#include "driver/uart.h"
#include "lib/types.h"
#include "kernel/console.h"
#include "kernel/global_channel.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
#include "kernel/resource_factory.h"
#include "kernel/lock/critical_guard.h"

namespace kernel {

Console::Console(driver::BasicDevice* device) : device_(device) {
  device->RegisterInterruptCallback([](const char* str, size_t len){
    auto* resource = kernel::ResourceFactory::Instance()->GetResource(
      kernel::resource_id::console_major,
      kernel::resource_id::console_minor);
    if (resource) {
      (reinterpret_cast<kernel::Console*>(resource))->InterruptHandler(str, len);
    }
  });
}

size_t Console::read(char* p, size_t len) {
  CriticalGuard guard(&context.lock);
  size_t read_cnt = 0;
  while (read_cnt < len) {
    while (context.read_index == context.write_index) {
      Schedueler::Instance()->Sleep(GlobalChannel::console_channel(), &context.lock);
    }
    char c = context.buf[(context.read_index % buf_len)];
    context.read_index++;
    c = c == '\r' ? '\n' : c;
    p[read_cnt++] = c;
    if (c == '\n') {
      break;
    }
  }
  return read_cnt;
}

size_t Console::write(char*p, size_t len) {
  if (!device_->IsWritable()) {
    return 0;
  }
  device_->Write(p, len);
  return len;
}

void Console::get_name(char* name) {
  const char* s = "Console";
  std::copy(s, s + strlen(s), name);
}

static void EchoChar(uint8_t c) {
  if (c == '\r') {
    kernel::printf("\n");
  } else {
    kernel::printf("%c", c);
  }
}

static void EchoBackSpace() {
  const char* str = "\b \b";
  kernel::printf(str);
}

void Console::InterruptHandler(const char* s, size_t len) {
  CriticalGuard guard(&context.lock);
  if ((context.write_index % buf_len - context.read_index % buf_len) == 0 && context.read_index != context.write_index) {
    kernel::printf("Warning: console buffer is full\n");
    return;
  }
  for (size_t i = 0; i < len; i++) {
    if (s[i] == '\x7f') {
      context.write_index--;
      EchoBackSpace();
      continue;
    }
    context.buf[context.write_index % buf_len] = s[i];
    context.write_index += 1;
    EchoChar(s[i]);
    bool is_buffer_full = (context.write_index % buf_len - context.read_index % buf_len) == 0 && context.read_index != context.write_index;
    if (s[i] == '\r' || is_buffer_full) {
      Schedueler::Instance()->Wakeup(GlobalChannel::console_channel());
      if (is_buffer_full) {
        return;
      }
    }
  }
}

}  // namespace kernel