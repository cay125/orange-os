#pragma once

#include "driver/virtio.h"
#include "lib/types.h"

namespace driver {
namespace virtio {

namespace input {

struct virtio_input_event {
  uint16_t type;
  uint16_t code;
  uint32_t value;
};

enum class virtio_input_config_select : uint8_t {
  VIRTIO_INPUT_CFG_UNSET     = 0x00,
  VIRTIO_INPUT_CFG_ID_NAME   = 0x01,
  VIRTIO_INPUT_CFG_ID_SERIAL = 0x02,
  VIRTIO_INPUT_CFG_ID_DEVIDS = 0x03,
  VIRTIO_INPUT_CFG_PROP_BITS = 0x10,
  VIRTIO_INPUT_CFG_EV_BITS   = 0x11,
  VIRTIO_INPUT_CFG_ABS_INFO  = 0x12,
};

struct virtio_input_absinfo {
  uint32_t min;
  uint32_t max;
  uint32_t fuzz;
  uint32_t flat;
  uint32_t res;
};

struct virtio_input_devids {
  uint16_t bustype;
  uint16_t vendor;
  uint16_t product;
  uint16_t version;
};

struct virtio_input_config {
  uint8_t select;
  uint8_t subsel;
  uint8_t size;
  uint8_t reserved[5];
  union {
    char string[128];
    uint8_t bitmap[128];
    virtio_input_absinfo abs;
    virtio_input_devids ids;
  } u;
};

struct Position {
  uint32_t x;
  uint32_t y;
};

}

class InputDevice : public DeviceViaMMIO {
 public:
  InputDevice();
  bool Init(uint64_t virtio_addr) override;
  device_id GetDeviceId() override;
  void UsedBufferNotify() override;
  void ConfigChangeNotify() override;

  input::Position GetPosition() const;

 private:
  input::virtio_input_config* config_ = nullptr;
  input::virtio_input_event input_event_[queue_buffer_size];
  input::Position position_{};
};

}
}
