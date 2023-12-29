#include "core/util/buffer/Buffer.h"

#include <cassert>
#include <cstring>

LY_NAMESPACE_BEGIN

Buffer::Buffer(size_t capacity) :
  capacity_(capacity)
{
  data_ = new char[capacity + 1];
}

Buffer::~Buffer()
{
  delete[] data_;
}

size_t Buffer::read(char *s, size_t n)
{
  // assert(sizeof(s) >= n);
  size_t bytesRead = readableBytes();
  if (bytesRead > n) {
    bytesRead = n;
  }

  if (s) {
    for (size_t i = 0; i < bytesRead; ++i)
      s[i] = data_[indexOf(get_pos_, i)];
  }

  get_pos_ = indexOf(get_pos_, bytesRead);
  return bytesRead;
}

size_t Buffer::write(const char *s, size_t n)
{
  // assert(sizeof(s) <= n);
  size_t bytesWrite = capacity_ - size();
  if (bytesWrite > n) {
    bytesWrite = n;
  }

  if (s) {
    for (size_t i = 0; i < bytesWrite; ++i)
      data_[indexOf(put_pos_, i)] = s[i];
  }

  put_pos_ = indexOf(put_pos_, bytesWrite);
  return bytesWrite;
}

int Buffer::read(sockfd_t fd)
{
  size_t len = writableBytes();
  if (len >= kMaxBytesPerRead) {
    len = kMaxBytesPerRead;
  }

  int bytesWrite = net::socket_api::recv(fd, data_ + put_pos_, len);
  if (bytesWrite > 0) {
    put_pos_ = indexOf(put_pos_, bytesWrite);
  }

  return bytesWrite;
}

int Buffer::write(sockfd_t fd, size_t size, int ms_timeout)
{
  if (ms_timeout > 0) net::socket_api::setBlocking(fd, ms_timeout);
  size_t len = readableBytes();
  if (size < len) {
    len = size;
  }

  int bytesRead = net::socket_api::send(fd, data_ + get_pos_, len);
  if (bytesRead > 0) {
    get_pos_ = indexOf(get_pos_, bytesRead);
  }

  if (ms_timeout > 0) net::socket_api::setNonBlocking(fd);
  return bytesRead;
}

void Buffer::reset(const size_t newCapacity)
{
  char *new_data = new char[newCapacity];
  if (nullptr == new_data) {
    throw std::runtime_error("no enough free memory");
  }

  size_t n = size() >= newCapacity ? newCapacity : size();
  for (size_t i = 0; i < n; ++i) {
    new_data[i] = data_[indexOf(get_pos_, i)];
  }

  delete[] data_;
  data_ = new_data;
  get_pos_ = 0;
  put_pos_ = n;
  capacity_ = newCapacity;
}

int Buffer::find(const char* target)
{
  int len = strlen(target);

  for (size_t j = 0; j < size(); ++j) {
    size_t i = 0;
    for (; i < len; ++i) {
      size_t idx = indexOf(j + get_pos_, i);
      if (idx == put_pos_) return -1;

      if (data_[idx] != target[i]) {
        break;
      }
    }
    if (i == len)
      return j;
  }

  return -1;
}

LY_NAMESPACE_END
