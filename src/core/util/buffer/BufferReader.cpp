#include "core/util/buffer/BufferReader.h"

LY_NAMESPACE_BEGIN

int BufferReader::append(const char *data, size_t len)
{
  int write_len = buffer_.writableBytes();
  if (write_len < Buffer::kMaxBytesPerRead) {
    buffer_.reset(buffer_.capacity() * 2);
  }

  buffer_.write(data, len);
  
  return len;
}
int BufferReader::read(size_t n, std::string &data)
{
  int bytesRead = buffer_.readableBytes();
  if (n > 0 && n <= bytesRead) {
    int len = buffer_.read(data.data(), n);
    return len;
  }

  return 0;
}

int BufferReader::findFirst(const char* matchStr)
{
  return buffer_.find(matchStr);
}

int BufferReader::findAndSkip(const char* matchStr)
{
  int len = buffer_.find(matchStr);
  if (len < 0) return len;

  buffer_.read(nullptr, len + strlen(matchStr));
  return len + strlen(matchStr);
}

int BufferReader::read(sockfd_t fd)
{
  int write_len = writableBytes();
  // ÈÝÁ¿²»×ã
  if (write_len < Buffer::kMaxBytesPerRead) {
    buffer_.reset(buffer_.capacity() * 2);
  }

  int len = buffer_.write(fd, Buffer::kMaxBytesPerRead);
  if (len <= 0) return 0;

  return len;
}

int BufferReader::readAll(std::string &data)
{
  int size = buffer_.readableBytes();
  buffer_.read(data.data(), size);

  return size;
}
void BufferReader::advance(size_t n)
{
  buffer_.read(nullptr, n);
}
void BufferReader::advanceTo(const char *target)
{
  int n = buffer_.find(target);
  buffer_.read(nullptr, n);
}
void BufferReader::clear()
{
  buffer_.clear();
}

uint16_t readU16Forward(const char *p)
{
  uint16_t res = 0;

  res |= p[0] << 8;
  res |= p[1];
  return res;
}
uint16_t readU16Reverse(const char *p)
{
  uint16_t res = 0;

  res |= p[1] << 8;
  res |= p[0];
  return res;
}
uint32_t readU24Forward(const char *p)
{
  uint32_t res = 0;

  res |= p[0] << 16;
  res |= p[1] << 8;
  res |= p[2];
  return res;
}
uint32_t readU24Reverse(const char *p)
{
  uint32_t res = 0;

  res |= p[2] << 16;
  res |= p[1] << 8;
  res |= p[0];
  return res;
}
uint32_t readU32Forward(const char *p)
{
  uint32_t res = 0;

  res |= p[0] << 24;
  res |= p[1] << 16;
  res |= p[2] << 8;
  res |= p[3];
  return res;
}
uint32_t readU32Reverse(const char *p)
{
  uint32_t res = 0;

  res |= p[3] << 24;
  res |= p[2] << 16;
  res |= p[1] << 8;
  res |= p[0];
  return res;
}

LY_NAMESPACE_END
