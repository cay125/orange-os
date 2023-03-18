// run on host machine

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "filesystem/inode_def.h"

size_t MaxSupportFileSize() {
  return (fs::DIRECT_ADDR_SIZE + fs::BLOCK_SIZE / sizeof(fs::InodeDef::addr[0])) * fs::BLOCK_SIZE;
}

int main(int argc, char* argv[]) {
  // 1 <-> img_name, 2 <-> img_size
  if (argc < 3) {
    std::cout << "Input param error\n";
    return -1;
  }
  std::ofstream fs_img(argv[1], std::ios::binary | std::ios::trunc);
  if (!fs_img.is_open()) {
    return -1;
  }
  if (fs::INODE_ELEMENT_SIZE != sizeof(fs::InodeDef) ||
      fs::DIR_ELEMENT_SIZE != sizeof(fs::DirElement) ||
      fs::BLOCK_SIZE % fs::INODE_ELEMENT_SIZE != 0) {
    std::cout << "filesystem configuration is not valid\n";
    return -1;
  }
  std::cout << "[fs] block_size: " << fs::BLOCK_SIZE
            << "\n[fs] dir_element size: " << fs::DIR_ELEMENT_SIZE
            << "\n[fs] inode_element_size: " << fs::INODE_ELEMENT_SIZE
            << "\n[fs] inode_table_size: " << fs::INODE_TABLE_SIZE
            << "\n[fs] max_support_file_size: " << MaxSupportFileSize() << " bytes"
            << "\n[fs] file_image size: " << std::stoi(argv[2]) * fs::BLOCK_SIZE << " bytes\n";

  // setup super block
  fs::SuperBlock super_block{};
  super_block.magic = *(reinterpret_cast<const uint32_t*>(fs::FSMAGIC));
  super_block.size = std::stoi(argv[2]);
  super_block.inode_start = 1;
  super_block.inodes_count = fs::INODE_TABLE_SIZE;
  super_block.bmap_start = super_block.inode_start + super_block.inodes_count * fs::INODE_ELEMENT_SIZE / fs::BLOCK_SIZE;
  super_block.datablocks_count = super_block.size - super_block.bmap_start - 1;

  // alloc img size
  for (int i = 0; i < super_block.size; ++i) {
    char buf[fs::BLOCK_SIZE] = {0};
    fs_img.seekp(i * fs::BLOCK_SIZE);
    fs_img.write(buf, sizeof(buf));
  }

  fs_img.seekp(0);
  fs_img.write(reinterpret_cast<char*>(&super_block), sizeof(fs::SuperBlock));
  
  uint32_t cur_data_block = super_block.bmap_start + 1;
  uint32_t cur_inode_index = 1;

  // prepare root_inode
  fs::InodeDef root_inode{};
  root_inode.type = fs::file_type::directory;
  root_inode.link_count = 1;

  for (int i = 3; i < argc; ++i) {
    if ((cur_data_block - super_block.bmap_start) > super_block.datablocks_count) {
      std::cout << "Reach fs_img max\n";
      break;
    }
    std::ifstream file(argv[i]);
    if (!file.is_open()) {
      std::cout << "Open file: " << argv[i] << " failed\n";
      continue;
    }
    file.seekg(0, std::ios_base::end);
    auto file_size = file.tellg();
    std::cout << "Procesing file: " << argv[i] << " file_size: " << file_size << "\n";

    if (file_size > MaxSupportFileSize()) {
      std::cout << "File_size too large, continued\n";
      continue;
    }

    std::filesystem::path path(argv[i]);
    std::string file_name = path.filename();
    if (file_name.length() >= fs::MAX_FILE_NAME_LEN) {
      std::cout << "File_name too long, continued\n";
      continue;
    }

    // setup inode
    fs::InodeDef inode{};
    inode.type = fs::file_type::regular_file;
    inode.size = file_size;
    inode.link_count = 1;

    fs::DirElement dir_element{};
    std::copy(file_name.begin(), file_name.end(), dir_element.name);
    dir_element.inum = cur_inode_index;
    auto cur_root_data_index = root_inode.size / fs::BLOCK_SIZE;
    if (!root_inode.addr[cur_root_data_index]) {
      root_inode.addr[cur_root_data_index] = cur_data_block;
      cur_data_block += 1;
    }
    auto cur_inode_data_offset = root_inode.size % fs::BLOCK_SIZE;
    fs_img.seekp(root_inode.addr[cur_root_data_index] * fs::BLOCK_SIZE + cur_inode_data_offset);
    fs_img.write(reinterpret_cast<char*>(&dir_element), sizeof(dir_element));

    root_inode.size += fs::DIR_ELEMENT_SIZE;

    // setup data block
    char buf[fs::BLOCK_SIZE] = {0};
    fs::IndirectBlock indirect_block;
    uint32_t cur_indirect_block_index = 0;
    std::streamsize cur_size = 0, total_processed_size = 0;
    file.seekg(0);
    while (file.read(buf, sizeof(buf)) && (cur_size = file.gcount())) {
      auto cur_data_index = total_processed_size / fs::BLOCK_SIZE;
      if (total_processed_size <= (fs::DIRECT_ADDR_SIZE * fs::BLOCK_SIZE)) {
        inode.addr[cur_data_index] = cur_data_block;
      } else {
        if (!inode.indirect_addr) {
          inode.indirect_addr = cur_data_block;
        }
        indirect_block[cur_indirect_block_index] = ++cur_data_block;
        cur_indirect_block_index += 1;
      }

      fs_img.seekp(cur_data_block * fs::BLOCK_SIZE);
      fs_img.write(buf, sizeof(buf));

      cur_data_block += 1;
      total_processed_size += cur_size;
    }

    fs_img.seekp(cur_inode_index * sizeof(fs::InodeDef) + super_block.inode_start * fs::BLOCK_SIZE);
    fs_img.write(reinterpret_cast<char*>(&inode), sizeof(inode));

    if (indirect_block[0] != 0) {
      fs_img.seekp(inode.indirect_addr * fs::BLOCK_SIZE);
      fs_img.write(reinterpret_cast<char*>(&indirect_block), sizeof(indirect_block));
    }

    cur_inode_index += 1;
  }

  fs_img.seekp(fs::ROOT_INODE * fs::INODE_ELEMENT_SIZE + super_block.inode_start * fs::BLOCK_SIZE);
  fs_img.write(reinterpret_cast<char*>(&root_inode), sizeof(root_inode));

  fs_img.close();

  return 0;
}

