#include "filesystem/common.h"

#include "driver/device_factory.h"
#include "driver/virtio.h"
#include "kernel/utils.h"
#include "lib/string.h"

namespace fs {

bool GetInode(uint32_t inode_index, fs::InodeDef* inode) {
  driver::virtio::MetaData meta_data;
  meta_data.block_index = inode_index * fs::INODE_ELEMENT_SIZE / fs::BLOCK_SIZE + 1;
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  device->Operate(driver::virtio::Operation::read, &meta_data);
  *inode = *reinterpret_cast<fs::InodeDef*>(meta_data.buf.data() + (inode_index * fs::INODE_ELEMENT_SIZE % fs::BLOCK_SIZE));
  return true;
}

int IteratorDir(fs::InodeDef* inode,
                driver::virtio::Device* device,
                const char* target_name) {
  uint32_t current_offset = 0;
  while (current_offset < inode->size) {
    uint32_t current_addr_index = current_offset / fs::BLOCK_SIZE;
    if (!inode->addr[current_addr_index]) {
      kernel::panic("Invalid inode");
    }
    driver::virtio::MetaData meta_data;
    meta_data.block_index = inode->addr[current_addr_index];
    device->Operate(driver::virtio::Operation::read, &meta_data);
    auto p = meta_data.buf.begin();
    while (current_offset < inode->size && (p != meta_data.buf.end())) {
      auto* dir = reinterpret_cast<fs::DirElement*>(p);
      if (strcmp(target_name, dir->name) == 0) {
        return dir->inum;
      }
      p += fs::DIR_ELEMENT_SIZE;
      current_offset += fs::DIR_ELEMENT_SIZE;
    }
  }
  return -1;
}

ssize_t Open(const char* path_name, InodeDef* inode) {
  if (!path_name || path_name[0] != '/') {
    return -1;
  }
  int path_level = -1;
  const char* paths[16] = {nullptr};
  const char* p = path_name;
  auto path_len = strlen(path_name);
  for (size_t i = 0; i < path_len;) {
    while (*p && *p != '/') {
      ++p;
    }
    if (*p && *(p + 1)) {
      ++p;
      paths[++path_level] = p;
    }
    i = p - path_name;
  }

  int current_inode_index = fs::ROOT_INODE;
  auto device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  for (int i = 0; i <= path_level; ++i) {
    InodeDef inode{};
    GetInode(current_inode_index, &inode);
    if (i != path_level && inode.type != fs::inode_type::directory) {
      return -1;
    }
    int inum = IteratorDir(&inode, device, paths[i]);
    if (inum < 0) {
      return -1;
    }
    current_inode_index = inum;
  }
  GetInode(current_inode_index, inode);
  return current_inode_index;
}

}  // namespace fs
