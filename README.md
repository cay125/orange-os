# orange-os

Operating system implemented using morden C++ with virtio driver running on riscv platform. For **no_std** environment.

## How to use

```bash

sudo apt-get install qemu-system-misc gcc-riscv64-linux-gnu (Skip it if you already have an environment)

mkdir build && cd build
cmake ..
make qemu-gui
```
