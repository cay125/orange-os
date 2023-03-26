#ifndef FILESYSTEM_FILE_DESCRIPTOR_H
#define FILESYSTEM_FILE_DESCRIPTOR_H

#include "filesystem/inode_def.h"
#include "lib/types.h"

namespace fs {

enum class FileType : uint8_t {
  none,
  disk_file,
  device,
};

struct FileDescriptor {
  FileType file_type = FileType::none;
  uint64_t offset = 0;
  uint32_t inode_index = 0;
  InodeDef inode{};
};

}  // namespace fs


#endif  // FILESYSTEM_FILE_DESCRIPTOR_H