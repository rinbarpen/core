#include "core/util/buffer/BufferWriter.h"

LY_NAMESPACE_BEGIN

void writeU16Forward(char *p, uint16_t value)
{
  p[0] = value >> 8;
  p[1] = value & 0xFF;
}
void writeU16Reverse(char *p, uint16_t value)
{
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}
void writeU24Forward(char *p, uint32_t value)
{
  p[0] = value >> 16;
  p[1] = value >> 8;
  p[2] = value & 0xFF;
}
void writeU24Reverse(char *p, uint32_t value)
{
  p[2] = value >> 16;
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}
void writeU32Forward(char *p, uint32_t value)
{
  p[0] = value >> 24;
  p[1] = value >> 16;
  p[2] = value >> 8;
  p[3] = value & 0xFF;
}
void writeU32Reverse(char *p, uint32_t value)
{
  p[3] = value >> 24;
  p[2] = value >> 16;
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}

bool BufferWriter::append(const char *data, uint32_t len)
{
  if (full()) return false;

  auto buffer = std::make_shared<Buffer>(len);
  buffer->write(data, len);
  q_.emplace(buffer);
  return true;
}

int BufferWriter::send(sockfd_t fd, int ms_timeout)
{
  if (empty()) return -1;

  auto buffer = std::move(q_.front()); q_.pop();
  size_t size = buffer->readableBytes();
  buffer->write(fd, size, ms_timeout);
  return size;
}

LY_NAMESPACE_END
