#include "user/graphic/image_viewer.h"
#include "filesystem/inode_def.h"
#include "lib/common.h"
#include "lib/lib.h"
#include "lib/stl/smart_pointer.h"
#include "lib/syscall.h"
#include "lib/stl/vector.h"

namespace graphic {

namespace detail {
namespace bmp {

enum class compression_type : uint32_t {
  no_compression = 0,
  bi_rle8 = 1,
  bi_rle4 = 2,
};

const char* compression_type_name[] = {
  [0] = "no_compression",
  [1] = "bi_rle8",
  [2] = "bi_rle4"
};

const char* GetCompressionTypeName(compression_type type) {
  return compression_type_name[lib::common::literal(type)];
}

struct Header
{
  struct {
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
  } file_header;
  struct {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    compression_type compression;
    uint32_t image_size;
    uint32_t x_pixel_per_meter;
    uint32_t y_pixel_per_meter;
    uint32_t color_used;
    uint32_t color_important;
  } bmp_header;
};

class bmpParser : public ImageParser {
 public:
  void Init(int fd, const fs::FileState* state) override {
    syscall::read(fd, reinterpret_cast<char*>(&header_), sizeof(header_));
    if (header_.bmp_header.planes != 1) {
      printf(PrintLevel::error, "Invalid 'planes' in bmp header, got: %d\n", header_.bmp_header.planes);
      return;
    }
    if (header_.file_header.data_offset < sizeof(header_)) {
      printf(PrintLevel::error, "Invalid 'data_offset' in file header, got: %d\n", header_.file_header.data_offset);
      return;
    }
    if (header_.file_header.file_size != state->size) {
      printf(PrintLevel::error, "Invalid 'file_size' in file header, %d vs %d\n", header_.file_header.file_size, state->size);
    }
    if (header_.bmp_header.compression == compression_type::no_compression && header_.bmp_header.bits_per_pixel >= 8) {
      auto image_size = ((header_.bmp_header.bits_per_pixel / 8) * header_.bmp_header.width + 3) / 4 * 4 * header_.bmp_header.height;
      if (image_size != header_.bmp_header.image_size) {
        printf(PrintLevel::error, "Invalid 'image_size' in bmp header, %d vs %d\n", image_size, header_.bmp_header.image_size);
        return;
      }
    }
    printf(PrintLevel::info,
           "img H: %d, W: %d, bits_per_pixel: %d, compression_method: %s\n",
           header_.bmp_header.height, header_.bmp_header.width, header_.bmp_header.bits_per_pixel, GetCompressionTypeName(header_.bmp_header.compression));
    is_valid_ = true;
    fd_ = fd;
  }

  bool DoParse(lib::vector<uint8_t>* data) override {
    if (!is_valid_) {
      return false;
    }
    uint32_t row_size = header_.bmp_header.width * header_.bmp_header.bits_per_pixel / 8;
    uint32_t img_size = header_.bmp_header.height * row_size;
    uint32_t padding = ((row_size) % 4) == 0 ? 0 : 4 - ((row_size) % 4);
    data->resize(img_size);
    for (uint32_t row_index = 0; row_index < header_.bmp_header.height; ++row_index) {
      uint32_t size = row_size;
      // pixels are stored bottom to top instead of top to bottom
      char* buf = reinterpret_cast<char*>(data->data() + (header_.bmp_header.height - row_index - 1) * row_size);
      while (size > 0) {
        constexpr size_t size_per_batch = 128;
        const size_t count = size > size_per_batch ? size_per_batch : size;
        syscall::read(fd_, buf, count);
        buf += count;
        size -= count;
      }
      if (padding) {
        char padding_buf[4];
        syscall::read(fd_, padding_buf, padding);
      }
    }
    return true;
  }

  uint32_t height() const override {
    if (!is_valid_) {
      return 0;
    }
    return header_.bmp_header.height;
  }

  uint32_t width() const override {
    if (!is_valid_) {
      return 0;
    }
    return header_.bmp_header.width;
  }

 private:
  Header header_;
  int fd_;
  bool is_valid_ = false;
};

}  // namespace bmp
}  // namespace detail

ImageViewer::ImageViewer(const char* path) {
  int fd = syscall::open(path);
  if (fd < 0) {
    printf(PrintLevel::error, "Cannot open target file: %s\n", path);
    return;
  }
  fs::FileState file_state;
  syscall::fstat(fd, &file_state);
  if (file_state.type != fs::FileType::regular_file) {
    printf(PrintLevel::error, "Target file: %s is not regular file\n", path);
    return;
  }
  char buf[2];
  syscall::read(fd, buf, 2);
  if (buf[0] == 'B' && buf[1] == 'M') {
    image_type_ = ImageType::bmp;
    image_parser_ = lib::make_unique<detail::bmp::bmpParser>();
    image_parser_->Init(fd, &file_state);
  } else {
    printf(PrintLevel::error, "Not supported file format\n");
  }
}

bool ImageViewer::Init() {
  if (!image_parser_) {
    return false;
  }
  image_parser_->DoParse(&data_);
  return true;
}

const uint8_t* ImageViewer::data() const  {
  return data_.data();
}

uint32_t ImageViewer::height() const {
  if (!image_parser_) {
    return 0;
  }
  return image_parser_->height();
}

uint32_t ImageViewer::width() const {
  if (!image_parser_) {
    return 0;
  }
  return image_parser_->width();
}

ImageType ImageViewer::image_type() const {
  return image_type_;
}

}  // namespace graphic
