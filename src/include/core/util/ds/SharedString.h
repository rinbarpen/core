#pragma once
#include <cstddef>
#include <cstring>
#include <memory>

#include <core/util/Traits.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
// use
template <typename CharT = char>
class SharedString final
{
  LY_CHECK((is_any_of_v<CharT, char, unsigned char, signed char>),
    "It should be char or unsigned char or signed char");
public:
  SharedString() : data_(nullptr), size_(0) {}
  explicit SharedString(size_t size)
    : data_(new CharT[size], std::default_delete<CharT>()), size_(size) {}
  ~SharedString() = default;

  void reset() { data_.reset(new CharT[size_], std::default_delete<CharT>()); }
  void reset(size_t newSize) {
    data_.reset(new CharT[newSize], std::default_delete<CharT>());
  }

  void fill(const CharT *data, size_t len, size_t startPos = 0) {
    for (size_t i = 0; i < len; ++i) {
      data_.get()[i + startPos] = data[i];
    }
  }

  CharT *get() const { return data_.get(); }

  static SharedString<CharT> fromString(const std::string &s) {
    SharedString<CharT> ss(s.length());
    ss.fill(static_cast<const CharT*>(s.c_str()), s.length());
    return ss;
  }
  static SharedString<CharT> fromString(const CharT *s, size_t len) {
    SharedString<CharT> ss(len);
    ss.fill(s, len);
    return ss;
  }
  std::string toString() const { return std::string(data_, size_); }

  CharT &operator[](size_t index) { return data_.get()[index]; }
  CharT &operator[](size_t index) const { return data_.get()[index]; }

  SharedString clone() {
    SharedString newStr(size_);
    newStr.fill(data_.get(), size_, 0);
    return newStr;
  }
  void resize(size_t newSize) {
    if (size_ < newSize) {
      std::shared_ptr<CharT> newData(
        new CharT[newSize], std::default_delete<CharT>());
      memcpy(newData.get(), data_.get(), size_);
      data_ = newData;
      size_ = newSize;
    }
    else {
      std::shared_ptr<CharT> newData(
        new CharT[newSize], std::default_delete<CharT>());
      memcpy(newData.get(), data_.get(), newSize);
      data_ = newData;
      size_ = newSize;
    }
  }
  void clear() { memset(data_.get(), 0, size_); }

  bool empty() const { return size_ == 0; }
  size_t size() const { return size_; }

private:
  std::shared_ptr<CharT> data_;
  size_t size_;
};

LY_NAMESPACE_END
