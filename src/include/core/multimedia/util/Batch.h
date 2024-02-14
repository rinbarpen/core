#pragma once
#include <vector>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
template <class T>
class Batch
{
public:
  Batch() = default;
  ~Batch() = default;

  auto unwarp() const noexcept -> std::vector<T>;

private:
  std::vector<T> dataset_;
};
LY_NAMESPACE_END
