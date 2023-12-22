#pragma once

#include "Inode.h"

enum class FileType { NONE, NORMAL, SOFT_LINK, HARD_LINK, };

class File 
{
public:
  
private:
  inode_id_t id_;
  FileType type_;

  std::string filename_;
  uint64_t file_size_;
  int64_t creation_time_;
  int64_t modification_time_;
  int64_t access_time_;
  std::vector<block_id_t> blocks_;
  char *file_content_;
};
