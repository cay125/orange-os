#include "kernel/sys_def/device_info.h"
#include "lib/lib.h"
#include "lib/smart_pointer.h"
#include "lib/syscall.h"
#include "user/graphic/image_viewer.h"
#include <array>
#include <utility>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf(PrintLevel::info, "usage: view image_name\n");
    return -1;
  }

  graphic::ImageViewer viewer(argv[1]);
  bool ret = viewer.Init();
  if (!ret) {
    return -1;
  }
  const uint8_t* data = viewer.data();

  device_info::screen_info screen_info;
  if (syscall::get_screen_info(&screen_info) < 0) {
    printf(PrintLevel::error, "get_screen_info failed\n");
    return -1;
  }
  printf(PrintLevel::info, "display resolution: [width: %d, height: %d]\n", screen_info.width, screen_info.height);

  auto* frame_buffer = syscall::framebuffer(0);
  const device_info::screen_info& c_screen_info = std::as_const(screen_info);
  for (uint32_t y = 0; y < viewer.height(); ++y) {
    if (y >= c_screen_info.height) {
      continue;
    }
    for (uint32_t x = 0; x < viewer.width(); ++x) {
      if (x >= c_screen_info.width) {
        continue;
      }
      frame_buffer[(y * c_screen_info.width + x) * 4 + 0] = data[(y * viewer.width() + x) * 3 + 2];
      frame_buffer[(y * c_screen_info.width + x) * 4 + 1] = data[(y * viewer.width() + x) * 3 + 1];
      frame_buffer[(y * c_screen_info.width + x) * 4 + 2] = data[(y * viewer.width() + x) * 3 + 0];
    }
  }
  syscall::frame_flush();
  return 0;
}