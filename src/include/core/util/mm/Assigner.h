#pragma once

#include <vector>

// TODO: implement me! For future, use jemalloc or tcmalloc. Jbuffer is also.
class Assigner
{
public:
  static constexpr int kBlockSize = 4096;

  auto assign(size_t size) -> char*;

private:
  std::vector<char*> blocks_;
};
