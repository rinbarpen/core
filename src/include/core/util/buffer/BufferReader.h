#pragma once

#include <cstdint>
#include <string_view>
#include <core/util/marcos.h>
#include <core/net/platform.h>

LY_NAMESPACE_BEGIN
class BufferReader
{
public:
  explicit BufferReader(uint32_t capacity)
    : capacity_(capacity)
  {
    data_ = new char[capacity];
  }
  ~BufferReader()
  {
    delete[] data_;
  }

  char * peek() { return data_ + get_pos_; }
  const char * peek() const { return data_ + get_pos_; }

  void read(char *data, uint32_t nbytes);
  std::string read(uint32_t nbytes);
  int readFromSocket(sockfd_t sockfd);
  std::string readAll();

  int append(std::string_view data);

  int findFirst(std::string_view matchStr);
  int findAndSkip(std::string_view matchStr);
  int findLast(std::string_view matchStr);

  void advance(int n);
  void advanceTo(const char *target);

  uint32_t readableBytes() const { return size(); }
  uint32_t writableBytes() const { return capacity() - size(); }

  void clear() { get_pos_ = put_pos_ = 0; }
	bool full() const { return size() == capacity_; }
	bool empty() const { return put_pos_ == get_pos_; }
  uint32_t size() const {
    return (capacity_ + 1 + put_pos_ - get_pos_) % (capacity_ + 1);
  }
  uint32_t capacity() const { return capacity_; }

  LY_NONCOPYABLE(BufferReader);
private:
  void advance(int &pos, int n);
  void reset();

private:
  int get_pos_{0};
  int put_pos_{0};
  uint32_t capacity_;
  char *data_{nullptr};
};

LY_NAMESPACE_END
