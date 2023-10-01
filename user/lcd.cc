#include "kernel/sys_def/device_info.h"
#include "lib/lib.h"
#include "lib/stl/smart_pointer.h"
#include "lib/syscall.h"
#include <array>

int main(int argc, char** argv) {
  device_info::screen_info screen_info;
  if (syscall::get_screen_info(&screen_info) < 0) {
    printf(PrintLevel::error, "get_screen_info failed\n");
    return -1;
  }
  printf(PrintLevel::info, "display resolution: [width: %d, height: %d]\n", screen_info.width, screen_info.height);
  auto* frame_buffer = syscall::framebuffer();
  for (uint32_t y = 0; y < screen_info.height; y++) {
    for (uint32_t x = 0; x < screen_info.width; x++) {
      frame_buffer[(y * screen_info.width + x) * 4] = x;
      frame_buffer[(y * screen_info.width + x) * 4 + 1] = 255;
      frame_buffer[(y * screen_info.width + x) * 4 + 2] = x + y;
    }
  }

  auto cursor_image = lib::make_unique<std::array<uint8_t, 64 * 64 * 4>>();
  for (int i = 0;i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      (*cursor_image)[(i * 64 + j) * 4] = 255;
      (*cursor_image)[(i * 64 + j) * 4 + 1] = 255;
      (*cursor_image)[(i * 64 + j) * 4 + 2] = 0;
      (*cursor_image)[(i * 64 + j) * 4 + 3] = 255;
    }
  }
  syscall::setup_cursor(cursor_image.get());
  syscall::frame_flush();

  int cnt = 0;
  while (true) {
    for (uint32_t y = 0; y < screen_info.height; y++) {
      for (uint32_t x = 0; x < screen_info.width; x++) {
        for (uint32_t z = 0; z < 4; z++) {
          frame_buffer[(y * screen_info.width + x) * 4 + z] = ((int)z == (cnt % 4)) ? 255 : 0;
        }
      }
    }
    syscall::frame_flush();
    syscall::sleep(100);
    cnt += 1;
  }
  syscall::detach_framebuffer();
  return 0;
}