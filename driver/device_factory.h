#ifndef DRIVER_DEVICE_FACTORY_H
#define DRIVER_DEVICE_FACTORY_H

#include <array>
#include <utility>

#include "lib/singleton.h"
#include "driver/virtio.h"

namespace driver {

enum class DeviceList : uint8_t {
  disk0,
};

class DeviceFactory : public lib::Singleton<DeviceFactory> {
 public:
  friend class lib::Singleton<DeviceFactory>;
  void InitDevices();
  driver::virtio::Device* GetDevice(DeviceList device);

 private:
  driver::virtio::BlockDevice blk_device0_;
};

}  // namespace driver

#endif  // DRIVER_DEVICE_FACTORY_H