#include <cstring>
#include <memory>

#include <core/util/marcos.h>
#include <core/net/SocketUtil.h>
#include <core/config/config.h>
#include <core/util/buffer/BufferReader.h>

LY_NAMESPACE_BEGIN
static auto g_max_buffer_size = LY_CONFIG_GET(common.buffer.max_buffer_size);

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

auto BufferReader::read(uint32_t nbytes) -> std::string {
  size_t bytesRead = this->readableBytes();
  if (nbytes > bytesRead) {
    nbytes = bytesRead;
  }
  
  char *out = new char[nbytes];
  std::memcpy(out, data_, nbytes);
  std::string r(out, nbytes);
  delete[] out;
  return r;
}
auto BufferReader::read(sockfd_t sockfd) -> int
{
  int writeBytes = writableBytes();
  if (writeBytes < g_max_buffer_size) {
    this->reset();
  }

  char *buffer = new char[g_max_buffer_size];
  int len = net::socket_api::recv(sockfd, buffer, writeBytes);
  if (len <= 0) return 0;

  std::memcpy(data_, buffer, len);
  delete[] buffer;
  return len;
}
auto BufferReader::readAll() -> std::string
{
  return this->read(this->readableBytes());
}

auto BufferReader::append(std::string_view data) -> int
{
  int writeBytes = this->writableBytes();
  if (writeBytes < g_max_buffer_size) {
    this->reset();
  }

  std::memcpy(peek(), data.data(), data.length());
  return data.length();
}

int BufferReader::findFirst(std::string_view matchStr)
{
  int begin = get_pos_;
  while (begin < put_pos_) {
    if (0 == std::strncmp(peek() + begin, matchStr.data(), matchStr.length())) {
      return begin - get_pos_;
    }
    advance(begin, 1);
  }
  return -1;
}
int BufferReader::findAndSkip(std::string_view matchStr)
{
  auto len = this->findFirst(matchStr);
  if (len < 0) {
    return -1;
  }
  this->advance(len + matchStr.length());

  return len + matchStr.length();
}
int BufferReader::findLast(std::string_view matchStr)
{
  int rbegin = put_pos_ - matchStr.length();
  while (rbegin >= get_pos_) {
    if (0 == std::strncmp(peek() + rbegin, matchStr.data(), matchStr.length())) {
      return rbegin - get_pos_;
    }
    --rbegin;
  }
  return -1;
}

void BufferReader::advance(int n)
{
  advance(get_pos_, n);
  if (put_pos_ <= get_pos_) {
    clear();
  }
}
void BufferReader::advanceTo(const char *target)
{
  int len = std::strlen(target);
  int begin = get_pos_;
  while (begin < put_pos_) {
    if (0 == ::strcmp(peek() + begin, target)) {
      // found
      get_pos_ = begin;
      return ;
    }
    advance(begin, 1);
  }

  // not found
}

void BufferReader::advance(int &pos, int n)
{
  pos += n;
}
void BufferReader::reset()
{
  char *buffer = new char[capacity_ * 2];
  std::uninitialized_copy_n(data_, capacity_, buffer);
  delete[] data_;
  data_ = buffer;
  capacity_ *= 2;
}
LY_NAMESPACE_END
