#pragma once

#include <vector>

class Assigner
{
public:
  static constexpr int kBlockSize = 4096;

  auto assign(size_t size) -> char*;

private:
  std::vector<char*> blocks_;
};
