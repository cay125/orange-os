#include "kernel/sys_def/device_info.h"
#include "lib/lib.h"
#include "lib/syscall.h"

int main(int argc, char** argv) {
  device_info::screen_info screen_info;
  if (syscall::get_screen_info(&screen_info) < 0) {
    printf(PrintLevel::error, "get_screen_info failed\n");
    return -1;
  }
  printf(PrintLevel::info, "display resolution: [width: %d, height: %d]\n", screen_info.width, screen_info.height);
  auto* frame_buffer = syscall::framebuffer(0);
  for (uint32_t y = 0; y < screen_info.height; y++) {
    for (uint32_t x = 0; x < screen_info.width; x++) {
      frame_buffer[(y * screen_info.width + x) * 4] = x;
      frame_buffer[(y * screen_info.width + x) * 4 + 1] = 255;
      frame_buffer[(y * screen_info.width + x) * 4 + 2] = x + y;
    }
  }
  syscall::frame_flush();
  return 0;
}