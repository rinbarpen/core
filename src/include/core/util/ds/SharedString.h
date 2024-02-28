#pragma once
#include <memory>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
template <typename CharT = char>
class SharedString final
{
public:
  SharedString();
  explicit SharedString(size_t capacity);
  ~SharedString();

  void reset();
  void reset(size_t newCapacity);

  void expand(size_t newCapacity);

  void append(const CharT *data, size_t len);
  // use for char and unsigned char and signed char
  void append(std::string_view data);
  void fill(const CharT *data, size_t len, size_t startPos);
  // use for char and unsigned char and signed char
  void fill(std::string_view data, size_t startPos);

  LY_NODISCARD CharT *get() const { return data_.get(); }

  CharT& operator[](size_t index) { return data_.get()[index]; }
  CharT& operator[](size_t index) const { return data_.get()[index]; }

  SharedString clone();
  void resize(size_t size);
  void clear();
  void resizeAndClear(size_t size);

  bool empty() const { return size_ == 0; }
  LY_NODISCARD size_t size() const { return size_; }
  LY_NODISCARD size_t capacity() const { return capacity_; }

private:
  std::shared_ptr<CharT> data_;
  size_t size_{0};
  size_t capacity_{0};
};
LY_NAMESPACE_END
