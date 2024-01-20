#pragma once
#include <core/util/marcos.h>
#include <memory>

LY_NAMESPACE_BEGIN
class SharedString final
{
public:
  SharedString();
  SharedString(size_t capacity);
  ~SharedString();

  void reset();
  void reset(size_t newCapacity);

  void expand(size_t newCapacity);

  void append(const char *data, size_t len);
  void append(std::string_view data);
  void fill(const char *data, size_t len, size_t startPos);
  void fill(std::string_view data, size_t startPos);

  LY_NODISCARD char *get() const { return data_.get(); }

  SharedString clone();
  void resize(size_t size);

  LY_NODISCARD size_t size() const { return size_; }
  LY_NODISCARD size_t capacity() const { return capacity_; }

private:
  std::shared_ptr<char[]> data_;
  size_t size_{0};
  size_t capacity_{0};
};
LY_NAMESPACE_END