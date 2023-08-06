#include "filesystem/inode_def.h"
#include "lib/common.h"

namespace fs {

const char* FileTypeStr[] = {
  [0] = "none",
  [1] = "directory",
  [2] = "regular_file",
  [3] = "device",
};

const char* FileTypeName(FileType type) {
  return FileTypeStr[lib::common::literal(type)];
}

}