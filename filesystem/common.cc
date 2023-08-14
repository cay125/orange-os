#include "filesystem/common.h"

#include <algorithm>
#include <utility>
#include "driver/device_factory.h"
#include "driver/virtio.h"
#include "kernel/printf.h"
#include "kernel/scheduler.h"
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

bool UpdateInode(uint32_t inode_index, fs::InodeDef* inode) {
  driver::virtio::MetaData meta_data;
  meta_data.block_index = inode_index * fs::INODE_ELEMENT_SIZE / fs::BLOCK_SIZE + 1;
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  device->Operate(driver::virtio::Operation::read, &meta_data);
  *reinterpret_cast<fs::InodeDef*>(meta_data.buf.data() + (inode_index * fs::INODE_ELEMENT_SIZE % fs::BLOCK_SIZE)) = *inode;
  device->Operate(driver::virtio::Operation::write, &meta_data);
  return true;
}

bool GetSuperNode(fs::SuperBlock* super_block) {
  driver::virtio::MetaData meta_data;
  meta_data.block_index = 0;
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  device->Operate(driver::virtio::Operation::read, &meta_data);
  *super_block = *reinterpret_cast< fs::SuperBlock*>(meta_data.buf.data());
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

static int ParsePathImpl(const char* p, char out_paths[][fs::MAX_FILE_NAME_LEN], bool is_relative = false) {
  int path_level = -1;
  const char* paths[16] = {nullptr};
  if (is_relative) {
    paths[++path_level] = p;
  }
  while (true) {
    while (*p && *p != '/') {
      ++p;
    }
    if (!*p || !*(p + 1)) {
      break;
    } else {
      ++p;
      // patten like: "xx//xxx"
      if (*p == '/') {
        return -1;
      }
      paths[++path_level] = p;
    }
  }
  for (auto i = 0; i <= path_level; i++) {
    size_t len = (i < path_level) ? paths[i + 1] - paths[i] : strlen(paths[i]);
    if (len >= fs::MAX_FILE_NAME_LEN) {
      return -1;
    }
    if (paths[i][len - 1] == '/') {
      len -= 1;
    }
    std::copy(paths[i], paths[i] + len, out_paths[i]);
  }
  return path_level + 1;
}

int ParsePath(const char* path_name, char out_paths[][fs::MAX_FILE_NAME_LEN]) {
  if (!path_name || strlen(path_name) == 0) {
    return -1;
  }
  if (path_name[0] != '/') {
    int level = ParsePathImpl(kernel::Schedueler::Instance()->ThisProcess()->current_path, out_paths);
    return ParsePathImpl(path_name, out_paths + level, true) + level;
  } else {
    return ParsePathImpl(path_name, out_paths);
  }
}

ssize_t OpenImpl(const char paths[][fs::MAX_FILE_NAME_LEN], int path_level, InodeDef* output_inode, int entry_inode = -1) {
  int current_inode_index = fs::ROOT_INODE;
  if (entry_inode >= 0) {
    current_inode_index = entry_inode;
  }
  auto device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  for (int i = 0; i < path_level; ++i) {
    InodeDef inode{};
    GetInode(current_inode_index, &inode);
    if (inode.type != fs::FileType::directory) {
      return -1;
    }
    int inum = IteratorDir(&inode, device, paths[i]);
    if (inum < 0) {
      return -1;
    }
    current_inode_index = inum;
  }
  GetInode(current_inode_index, output_inode);
  return current_inode_index;
}

ssize_t Open(const char* path_name, InodeDef* inode) {
  char paths[16][fs::MAX_FILE_NAME_LEN] = {0};
  int path_level = ParsePath(path_name, paths);
  if (path_level < 0) {
    return -1;
  }
  return OpenImpl(paths, path_level, inode);
}

std::pair<bool, uint32_t> AllocDataBlock() {
  fs::SuperBlock super_block{};
  GetSuperNode(&super_block);
  driver::virtio::MetaData meta_data;
  meta_data.block_index = super_block.bmap_start;
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  device->Operate(driver::virtio::Operation::read, &meta_data);
  int64_t target_block_index = -1;
  for (size_t i = 0; i < meta_data.buf.size(); i++) {
    if ((i * 8) >= super_block.datablocks_count) {
      break;
    }
    if (meta_data.buf[i] != 0xff) {
      uint8_t bit_mask[] = {0x01, 0x01 << 1, 0x01 << 2, 0x01 << 3, 0x01 << 4, 0x01 << 5, 0x01 << 6,0x01 << 7};
      for (int bit_index = 0; bit_index < 8; bit_index++) {
        if ((bit_mask[bit_index] & meta_data.buf[i]) == 0) {
          target_block_index = i * 8 + bit_index;
          meta_data.buf[i] |= bit_mask[bit_index];
          break;
        }
      }
      if (target_block_index >= 0) {
        break;
      }
    }
  }
  if (target_block_index < 0 || target_block_index >= super_block.datablocks_count) {
    kernel::printf("Error: No free data_block\n");
    return {false, 0};
  }
  device->Operate(driver::virtio::Operation::write, &meta_data);
  return {true, target_block_index + super_block.bmap_start + 1};
}

std::pair<bool, uint32_t> AllocInode() {
  fs::SuperBlock super_block{};
  GetSuperNode(&super_block);
  driver::virtio::MetaData meta_data;
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  for (uint32_t i = 0; i < (super_block.inodes_count * fs::INODE_ELEMENT_SIZE / fs::BLOCK_SIZE); i++) {
    meta_data.block_index = super_block.inode_start + i;
    device->Operate(driver::virtio::Operation::read, &meta_data);
    auto* inode = reinterpret_cast<InodeDef*>(meta_data.buf.data());
    for (uint32_t j = 0; j < (fs::BLOCK_SIZE / fs::INODE_ELEMENT_SIZE); j++) {
      if (inode[j].type == FileType::none) {
        return {true, i * (fs::BLOCK_SIZE / fs::INODE_ELEMENT_SIZE) + j};
      }
    }
  }
  return {false, 0};
}

std::pair<bool, uint32_t> CreateImpl(uint32_t inode_index, FileType type, const char* name, uint8_t major, uint8_t minor) {
  if (strlen(name) > fs::MAX_FILE_NAME_LEN) {
    return {false, 0};
  }
  InodeDef inode{};
  GetInode(inode_index, &inode);
  if (inode.type != FileType::directory) {
    return {false, 0};
  }
  auto* device = reinterpret_cast<driver::virtio::Device*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0));
  uint32_t addr_index = inode.size / fs::BLOCK_SIZE;
  uint32_t target_data_block_index = 0;
  auto new_inode_pair = AllocInode();
  if (!new_inode_pair.first) {
    return {false, 0};
  }
  if (addr_index < fs::DIRECT_ADDR_SIZE) {
    if (!inode.addr[addr_index]) {
      auto new_block = AllocDataBlock();
      if (!new_block.first) {
        return {false, 0};
      }
      inode.addr[addr_index] = new_block.second;
    }
    target_data_block_index = inode.addr[addr_index];
  } else {
    if (!inode.indirect_addr) {
      auto indirect_block_index = AllocDataBlock();
      auto data_block_index = AllocDataBlock();
      if (!indirect_block_index.first || !data_block_index.first) {
        return {false, 0};
      }
      inode.indirect_addr = indirect_block_index.second;
      driver::virtio::MetaData meta_data{};
      meta_data.block_index = indirect_block_index.second;
      std::fill(meta_data.buf.begin(), meta_data.buf.end(), 0);
      (reinterpret_cast<uint32_t*>(meta_data.buf.data()))[0] = data_block_index.second;
      device->Operate(driver::virtio::Operation::write, &meta_data);
      target_data_block_index = data_block_index.second;
    } else {
      driver::virtio::MetaData meta_data{};
      meta_data.block_index = inode.indirect_addr;
      device->Operate(driver::virtio::Operation::read, &meta_data);
      target_data_block_index = reinterpret_cast<uint32_t*>(meta_data.buf.data())[addr_index - fs::DIRECT_ADDR_SIZE];
    }
  }
  auto remain = inode.size % fs::BLOCK_SIZE;
  driver::virtio::MetaData meta_data{};
  meta_data.block_index = target_data_block_index;
  device->Operate(driver::virtio::Operation::read, &meta_data);
  DirElement dir{};
  dir.inum = new_inode_pair.second;
  std::copy(name, name + strlen(name), dir.name);
  reinterpret_cast<DirElement*>(meta_data.buf.data())[remain / fs::DIR_ELEMENT_SIZE] = dir;
  device->Operate(driver::virtio::Operation::write, &meta_data);
  inode.size += fs::DIR_ELEMENT_SIZE;
  UpdateInode(inode_index, &inode);

  InodeDef target_inode{};
  target_inode.size = 0;
  if (type == FileType::device) {
    target_inode.major = major;
    target_inode.minor = minor;
  }
  target_inode.type = type;
  target_inode.link_count = 1;
  UpdateInode(new_inode_pair.second, &target_inode);
  return {true, new_inode_pair.second};
}

bool Create(const char* path_name, FileType type, uint8_t major, uint8_t minor) {
  char paths[16][fs::MAX_FILE_NAME_LEN] = {0};
  int path_level = ParsePath(path_name, paths);
  if (path_level < 0) {
    kernel::printf("Error: got wrong file path\n");
    return false;
  }
  InodeDef inode{};
  int entry_inode = fs::ROOT_INODE;
  for (int i = 0; i < path_level; i++) {
    auto open_ret = OpenImpl(paths + i, 1, &inode, entry_inode);
    if (open_ret < 0) {
      auto create_ret = CreateImpl(entry_inode, (i == (path_level - 1)) ? type : FileType::directory, paths[i], major, minor);
      if (!create_ret.first) {
        kernel::printf("Error: Create file failed, target: \"%s\"\n", paths + i);
        return false;
      }
      entry_inode = create_ret.second;
    } else {
      if (i == (path_level - 1)) {
        kernel::printf("Warning: Target File: \"%s\" exists, type: %s, size: %d\n", path_name, FileTypeName(inode.type), inode.size);
        if (inode.type != type) {
          return false;
        }
      }
      entry_inode = open_ret;
    }
  }
  return true;
}

}  // namespace fs
