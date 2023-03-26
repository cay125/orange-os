#include "filesystem/common.h"

#include "driver/device_factory.h"
#include "driver/virtio.h"

namespace fs {

bool GetInode(uint32_t inode_index, fs::InodeDef* inode) {
  driver::virtio::MetaData meta_data;
  meta_data.block_index = inode_index * fs::INODE_ELEMENT_SIZE / fs::BLOCK_SIZE + 1;
  auto* device = driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0);
  device->Operate(driver::virtio::Operation::read, &meta_data);
  *inode = *reinterpret_cast<fs::InodeDef*>(meta_data.buf.data() + (inode_index * fs::INODE_ELEMENT_SIZE % fs::BLOCK_SIZE));
  return true;
}

}  // namespace fs