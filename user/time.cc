#include "lib/lib.h"
#include "lib/syscall.h"

class TimeMonitor {
 public:
  TimeMonitor() : tick_(syscall::uptime()) {}
  ~TimeMonitor() {
    if (!need_output_) {
      return;
    }
    auto cur = syscall::uptime();
    printf(PrintLevel::info, "Total use time tick: %d\n", cur - tick_);
  }
  void disable() {
    need_output_ = false;
  }
 private:
  int tick_;
  bool need_output_ = true;
};

int main(int argc, char** argv) {
  if (argc == 1) {
    printf(PrintLevel::error, "time: missing operand\nTry 'time --help' for more information.\n");
    return -1;
  }
  {
    TimeMonitor monitor;
    int sub_process = syscall::fork();
    if (sub_process < 0) {
      printf("trying to create a new process failed\n");
      return -1;
    }
    if (sub_process == 0) {
      syscall::exec(argv[1], argv + 1);
      printf("%s: command not found\n", argv[1]);
      monitor.disable();
      return -1;
    }
    syscall::wait();
  }
  return 0;
}
