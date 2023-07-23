#ifndef FILESYSTEM_COMMON_H
#define FILESYSTEM_COMMON_H

#include "filesystem/file_descriptor.h"
#include "filesystem/inode_def.h"

namespace fs {

bool GetInode(uint32_t inode_index, fs::InodeDef* inode);
ssize_t Open(const char* path, fs::InodeDef* inode);
bool Create(const char* path, FileType type, uint8_t major = 0, uint8_t minor = 0);

}  // namespace fs

#endif