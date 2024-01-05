#pragma once

#include <string>
#include <memory>
#include <cstring>

#include "core/util/buffer/Buffer.h"

LY_NAMESPACE_BEGIN

uint16_t readU16Forward(const char *p);
uint16_t readU16Reverse(const char *p);
uint32_t readU24Forward(const char *p);
uint32_t readU24Reverse(const char *p);
uint32_t readU32Forward(const char *p);
uint32_t readU32Reverse(const char *p);

class BufferReader
{
public:
  using ptr = std::shared_ptr<BufferReader>;

  BufferReader(size_t init_size = Buffer::kMaxBytesPerRead) :
    buffer_(init_size)
  {}

  virtual ~BufferReader() {}

  char *peek() { return buffer_.peek(); }
  const char *peek() const { return buffer_.peek(); }

  int append(const char *data, size_t len);
  int read(size_t n, std::string& data);

  /**
   * \brief 
   * \param matchStr 
   * \return the bytes we step by
   */
  int findFirst(const char *matchStr);
  int findAndSkip(const char *matchStr);

  int read(sockfd_t fd);
  int readAll(std::string &data);

  void advance(size_t n);
  void advanceTo(const char *target);
  void clear();

  size_t capacity() const { return buffer_.capacity(); }
  size_t readableBytes() const { return buffer_.readableBytes(); }
  size_t writableBytes() const { return buffer_.writableBytes(); }

protected:
  Buffer buffer_;
};

LY_NAMESPACE_END
