#pragma once

#include "filesystem/inode_def.h"
#include "lib/smart_pointer.h"
#include "lib/types.h"
#include "lib/vector.h"

namespace graphic {

namespace detail {
class ImageParser {
 public:
  virtual void Init(int fd, const fs::FileState* state) = 0;
  virtual bool DoParse(lib::vector<uint8_t>* data) = 0;
  virtual uint32_t height() const = 0;
  virtual uint32_t width() const = 0;
  virtual ~ImageParser() {}
};
}

enum class ImageType {
  none,
  bmp,
  jpeg,
};

class ImageViewer {
 public:
  ImageViewer(const char* path);
  bool Init();
  const uint8_t* data() const;
  uint32_t height() const;
  uint32_t width() const;
  uint32_t depth() const;
  uint8_t bits_per_pixel() const;
  ImageType image_type() const;

 private:
  lib::unique_ptr<detail::ImageParser> image_parser_;
  lib::vector<uint8_t> data_;
  ImageType image_type_ = ImageType::none;
};

}  // namespace graphic
