#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <array>
#include "lib/types.h"

namespace fs {

constexpr uint32_t BLOCK_SIZE = 512;
constexpr uint32_t DIRECT_ADDR_SIZE = 12;
constexpr uint32_t DIR_ELEMENT_SIZE = 16;
constexpr uint32_t INODE_ELEMENT_SIZE = 64;
constexpr uint32_t INODE_TABLE_SIZE = 320;
constexpr uint32_t MAX_FILE_NAME_LEN = 14;
constexpr uint32_t ROOT_INODE = 0;

using IndirectBlock = std::array<uint32_t, fs::BLOCK_SIZE / sizeof(uint32_t)>;

constexpr const char* FSMAGIC = "oran";

struct SuperBlock {
  uint32_t magic;              // Must be FSMAGIC
  uint32_t size;               // Size of file system image (blocks)
  uint32_t datablocks_count;   // Number of data blocks
  uint32_t inodes_count;       // Number of inodes
  uint32_t inode_start;        // Block number of first inode block
  uint32_t bmap_start;         // Block number of first free map block
};

struct DirElement {
  uint16_t inum;
  char name[MAX_FILE_NAME_LEN];
};

enum class FileType : uint8_t {
  none,
  directory,
  regular_file,
  device,
};

const char* FileTypeName(FileType type);

struct FileState {
  uint32_t inode_index;
  uint8_t link_count;
  uint32_t size;
  FileType type;
};

// disable struct align
#pragma  pack(1)
struct InodeDef {
  uint32_t inode_index;
  FileType type;
  uint8_t link_count;
  uint8_t major;
  uint8_t minor;
  uint32_t size;
  uint32_t addr[DIRECT_ADDR_SIZE];
  uint32_t indirect_addr;
};
#pragma  pack()

}  // namespace fs

#endif  // FILESYSTEM_H