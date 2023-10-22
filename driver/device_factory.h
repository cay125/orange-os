#ifndef DRIVER_DEVICE_FACTORY_H
#define DRIVER_DEVICE_FACTORY_H

#include <array>
#include <utility>

#include "driver/basic_device.h"
#include "driver/uart.h"
#include "driver/virtio.h"
#include "driver/virtio_blk.h"
#include "driver/virtio_gpu.h"
#include "driver/virtio_input.h"
#include "driver/virtio_net.h"
#include "lib/singleton.h"

namespace driver {

enum class DeviceList {
  device_list_begin,
  disk0,
  disk1,
  gpu0,
  mouse0,
  net0,
  uart0,
  device_list_end,
};

class DeviceFactory : public lib::Singleton<DeviceFactory> {
 public:
  friend class lib::Singleton<DeviceFactory>;
  void InitDevices();
  driver::BasicDevice* GetDevice(DeviceList device);

 private:
  driver::virtio::BlockDevice blk_device0_;
  driver::virtio::BlockDevice blk_device1_;
  driver::virtio::GPUDevice gpu_device0_;
  driver::virtio::InputDevice input_device0_;
  driver::virtio::NetDevice net0_;
  driver::Uart uart_;
};

}  // namespace driver

#endif  // DRIVER_DEVICE_FACTORY_H