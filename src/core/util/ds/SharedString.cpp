#include <core/util/ds/SharedString.h>

#include <cstring>
#include <memory>

SharedString::SharedString()
{

}

SharedString::SharedString(size_t capacity)
  : capacity_(capacity), data_(new char[capacity])
{
}

SharedString::~SharedString()
{
}

void SharedString::reset()
{
  data_.reset(new char[capacity_]);
  size_ = 0;
}

void SharedString::reset(size_t newCapacity)
{
  std::shared_ptr<char[]> new_data{new char[capacity_]};
  data_ = new_data;
  capacity_ = newCapacity;
  size_ = 0;
}

void SharedString::resize(size_t size)
{
  size_ = size;
}

void SharedString::expand(size_t newCapacity)
{
  std::shared_ptr<char[]> new_data{new char[capacity_]};
  if (size_ > newCapacity)
    size_ = newCapacity;
  memcpy(new_data.get(), data_.get(), size_);
  data_ = new_data;
  capacity_ = newCapacity;
}

void SharedString::append(const char* data, size_t len)
{
  if (capacity_ - size_ < len) {
    capacity_ = size_ + len;
    std::shared_ptr<char[]> new_data{new char[capacity_]};
    memcpy(new_data.get(), data_.get(), size_);
    memcpy(new_data.get() + size_, data, len);
    size_ += len;
    data_ = new_data;
  } else {
    memcpy(data_.get() + size_, data, len);
    size_ += len;
  }
}

SharedString SharedString::clone()
{
  auto cloned = SharedString(capacity_);
  cloned.append(data_.get(), size_);
  return cloned;
}
