#ifndef FILESYSTEM_COMMON_H
#define FILESYSTEM_COMMON_H

#include "filesystem/inode_def.h"

namespace fs {

bool GetInode(uint32_t inode_index, fs::InodeDef* inode);
ssize_t Open(const char* path, fs::InodeDef* inode);

}  // namespace fs

#endif