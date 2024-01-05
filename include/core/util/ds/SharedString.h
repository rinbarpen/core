#pragma once
#include <memory>

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

  char *get() const { return data_.get(); }

  SharedString clone();
  void resize(size_t size);

  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

private:
  std::shared_ptr<char[]> data_;
  size_t size_{0};
  size_t capacity_{0};
};
