#include <cstring>
#include <memory>

#include <core/util/ds/SharedString.h>
#include <type_traits>
#include "core/util/Traits.h"

LY_NAMESPACE_BEGIN
template <typename CharT>
SharedString<CharT>::SharedString()
{
}
template <typename CharT>
SharedString<CharT>::SharedString(size_t capacity)
  : data_(new char[capacity], std::default_delete<char[]>()), capacity_(capacity) {
}
template <typename CharT>
SharedString<CharT>::~SharedString()
{
}

template <typename CharT>
void SharedString<CharT>::reset()
{
  data_.reset(new char[capacity_]);
  size_ = 0;
}

template <typename CharT>
void SharedString<CharT>::reset(size_t newCapacity)
{
  std::shared_ptr<char> new_data(new char[capacity_], std::default_delete<char[]>());
  data_ = new_data;
  capacity_ = newCapacity;
  size_ = 0;
}

template <typename CharT>
void SharedString<CharT>::resize(size_t size)
{
  if (size > capacity_) {
    this->reset(size);
  }
  size_ = size;
}

template <typename CharT>
void SharedString<CharT>::resizeAndClear(size_t size)
{
  this->resize(size);
  memset(data_.get(), 0, size);
}

template <typename CharT>
void SharedString<CharT>::clear()
{
  memset(data_.get(), 0, size_);
}

template <typename CharT>
void SharedString<CharT>::expand(size_t newCapacity)
{
  std::shared_ptr<char> new_data(new char[capacity_], std::default_delete<char[]>());
  if (size_ > newCapacity)
    size_ = newCapacity;
  memcpy(new_data.get(), data_.get(), size_);
  data_ = new_data;
  capacity_ = newCapacity;
}

template <typename CharT>
void SharedString<CharT>::append(const CharT* data, size_t len)
{
  if (capacity_ - size_ < len) {
    capacity_ = size_ + len;
    std::shared_ptr<char> new_data(new char[capacity_], std::default_delete<char[]>());
    memcpy(new_data.get(), data_.get(), size_);
    memcpy(new_data.get() + size_, data, len);
    size_ += len;
    data_ = new_data;
  } else {
    memcpy(data_.get() + size_, data, len);
    size_ += len;
  }
}
template <typename CharT>
void SharedString<CharT>::append(std::string_view data)
{
  if constexpr (is_any_of_v<CharT, char, unsigned char, signed char>)
    this->append(data.data(), data.length());
}
template <typename CharT>
void SharedString<CharT>::fill(const CharT *data, size_t len, size_t startPos)
{
  if (startPos + len > size_) {
    return;
  }

  memcpy(data_.get() + startPos, data, len);
}
template <typename CharT>
void SharedString<CharT>::fill(std::string_view data, size_t startPos)
{
  if constexpr (is_any_of_v<CharT, char, unsigned char, signed char>)
    this->fill(data.data(), data.length(), startPos);
}

template <typename CharT>
SharedString<CharT> SharedString<CharT>::clone()
{
  auto cloned = SharedString<CharT>(capacity_);
  cloned.append(data_.get(), size_);
  return cloned;
}
LY_NAMESPACE_END
