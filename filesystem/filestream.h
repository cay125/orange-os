#ifndef FILESYSTEM_FILESTREAM_H
#define FILESYSTEM_FILESTREAM_H

#include "driver/virtio_blk.h"
#include "filesystem/inode_def.h"
#include "lib/streambase.h"

namespace fs {

class FileStream : public lib::StreamBase {
 public:
  FileStream(InodeDef& inode);
  FileStream(const FileStream&) = delete;
  FileStream& operator= (const FileStream&) = delete;

  virtual void Seek(size_t pos) override;
  virtual size_t Read(char* buf, size_t size) override;
  virtual size_t Size() const override;

  size_t write(const char* buf, size_t size);

 private:
  driver::virtio::BlockDevice* device_;
  fs::InodeDef& inode_;
};

}  // namespace fs

#endif  // namespace fs