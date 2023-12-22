#pragma once

#include <core/util/marcos.h>

using inode_id_t = uint32_t;
inline constexpr inode_id_t kErrorInodeId = -1;

class Inode 
{
public:
  SHARED_PTR_USING(Inode, ptr);

  Inode();

  static auto newInode() -> Inode::ptr { return std::make_shared<Inode>(); }

private:
  static inode_id_t hash() { return reinterpret_cast<uintptr_t>(this) & 0xFFFF; }

private:
  inode_id_t id_{ kErrorInodeId };
  
  Inode::ptr parent_;
  std::string path_;
};
