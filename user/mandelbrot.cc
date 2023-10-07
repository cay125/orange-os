#include "kernel/sys_def/device_info.h"
#include "lib/lib.h"
#include "lib/syscall.h"

class complex {
 public:
  complex(double real, double imag) : real_(real), imag_(imag) {}
  double norm() {
    return real_ * real_ + imag_ * imag_;
  }
  complex operator+(complex c) {
    return {this->real_ + c.real_, this->imag_ + c.imag_};
  }
  complex operator-(complex c) {
    return {this->real_ - c.real_, this->imag_ - c.imag_};
  }
  complex operator*(complex c) {
    return {this->real_ * c.real_ - this->imag_ * c.imag_, this->imag_ * c.real_ + this->real_ * c.imag_};
  }
  double real() {return real_;}
  double imag() {return imag_;}

 private:
  double real_;
  double imag_;
};

class FrameBufferGuard {
 public:
  FrameBufferGuard() {
    buffer_ = syscall::framebuffer();
    screen_info_ = get_screen_info();
  }

  ~FrameBufferGuard() {
    syscall::detach_framebuffer();
  }

  void mandle(complex c, complex t, int counter) {
    auto set_pixel = [this](uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b){
      if (y >= screen_info_.height || x >= screen_info_.width) {
        return;
      }
      if (y < 0 || x < 0) {
        return;
      }
      buffer_[(y * screen_info_.width + x) * 4 + 0] = r;
      buffer_[(y * screen_info_.width + x) * 4 + 1] = g;
      buffer_[(y * screen_info_.width + x) * 4 + 2] = b;
    };

    auto min_edge = std::min(screen_info_.height, screen_info_.width);
    if (t.norm() > 4) {
      int x = c.real() * min_edge / 3 + screen_info_.width / 2;
      int y =  c.imag() * min_edge / 3 + screen_info_.height / 2;
      set_pixel(
        x,
        y,
        255 - 128 * c.norm() / 4,
        255 - 128 * c.norm() / 4,
        255 - 128 * c.norm() / 4
      );
      return;
    }

    if (counter == 60) {
      int x = c.real() * min_edge / 3 + screen_info_.width / 2;
      int y =  c.imag() * min_edge / 3 + screen_info_.height / 2;
      set_pixel(x, y, 255.0 * ((t * t).norm() / ((t - c) * c).norm()), 0, 0);
      return;
    }

    mandle(c, t * t + c, counter + 1);
  }

  void flush() {
    syscall::frame_flush();
  }

  device_info::screen_info get_screen_info() {
    device_info::screen_info screen_info;
    syscall::get_screen_info(&screen_info);
    return screen_info;
  }

 private:
  device_info::screen_info screen_info_;
  uint8_t* buffer_; 
};


void waiting() {
  printf(PrintLevel::info, "press any key to return\n");
  int fd = syscall::open("/dev/console");
  char buf[1];
  syscall::read(fd, buf, 1);
}

int main(int argc, char** argv) {
  FrameBufferGuard frame;
  for (double x = -2; x < 2; x += 0.0015) {
    for (double y = -2; y < 2; y += 0.0015) {
      frame.mandle({x, y}, {0, 0}, 0);
    }
  }
  frame.flush();
  waiting();
  return 0;
}
