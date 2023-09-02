#ifndef DRIVER_DEVICE_FACTORY_H
#define DRIVER_DEVICE_FACTORY_H

#include <array>
#include <utility>

#include "lib/singleton.h"
#include "driver/basic_device.h"
#include "driver/uart.h"
#include "driver/virtio.h"
#include "driver/virtio_gpu.h"

namespace driver {

enum class DeviceList {
  device_list_begin,
  disk0,
  disk1,
  gpu0,
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
  driver::Uart uart_;
};

}  // namespace driver

#endif  // DRIVER_DEVICE_FACTORY_H