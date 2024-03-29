#include "filesystem/filestream.h"

#include "driver/device_factory.h"
#include "filesystem/common.h"
#include "filesystem/inode_def.h"
#include "lib/string.h"

namespace fs {

static uint64_t ConvertFsBlockToDriveBlock(uint64_t fs_block) {
  return fs_block * fs::BLOCK_SIZE / driver::virtio::block_size;
}

FileStream::FileStream(InodeDef& inode) : device_(reinterpret_cast<driver::virtio::BlockDevice*>(driver::DeviceFactory::Instance()->GetDevice(driver::DeviceList::disk0))), inode_(inode) {
  
}

void FileStream::Seek(size_t pos) {
  position_ = pos;
}

size_t FileStream::Read(char* buf, size_t size) {
  size = std::min(inode_.size - position_, size);
  auto total_remain_size = size;
  IndirectBlock indirect_block{};
  while (total_remain_size > 0) {
    size_t remain_size = fs::BLOCK_SIZE - position_ % fs::BLOCK_SIZE;
    size_t len = total_remain_size > remain_size ? remain_size : total_remain_size;
    auto data_block_index = position_ / fs::BLOCK_SIZE;
    if (data_block_index >= DIRECT_ADDR_SIZE) {
      if (!indirect_block[0]) {
        driver::virtio::MetaData data_block_data{indirect_block.data(), ConvertFsBlockToDriveBlock(inode_.indirect_addr)};
        device_->Operate<driver::virtio::Operation::read>(&data_block_data);
      }
    }
    uint64_t write_block_index = ConvertFsBlockToDriveBlock(data_block_index < DIRECT_ADDR_SIZE ? inode_.addr[data_block_index] : indirect_block[data_block_index - DIRECT_ADDR_SIZE]);
    if (len == BLOCK_SIZE) {
      driver::virtio::MetaData meta_data{buf, write_block_index};
      device_->Operate<driver::virtio::Operation::read>(&meta_data);
    } else {
      uint8_t sector_data[BLOCK_SIZE];
      driver::virtio::MetaData meta_data{sector_data, write_block_index};
      device_->Operate<driver::virtio::Operation::read>(&meta_data);
      memcpy(buf, meta_data.buf + (position_ % fs::BLOCK_SIZE), len);
    }
    total_remain_size -= len;
    position_ += len;
    buf += len;
  }
  return size;
}

size_t FileStream::write(const char* buf, size_t size) {
  auto write_cnt = Write(&inode_, buf, size, position_);
  return write_cnt;
}

size_t FileStream::Size() const {
  return inode_.size;
}

}