#include <cstring>

#include <core/util/marcos.h>
#include <core/net/SocketUtil.h>
#include <core/config/config.h>
#include <core/util/buffer/BufferReader.h>

LY_NAMESPACE_BEGIN
static auto g_max_buffer_size = LY_CONFIG_GET(common.buffer.max_buffer_size);

size_t BufferReader::read(char *data, uint32_t nbytes)
{
  size_t bytesRead = this->readableBytes();
  if (nbytes > bytesRead) {
    nbytes = bytesRead;
  }

  ::memcpy(data, data_, nbytes);
  return nbytes;
}
std::string BufferReader::read(uint32_t nbytes) {
  size_t bytesRead = this->readableBytes();
  if (nbytes > bytesRead) {
    nbytes = bytesRead;
  }

  std::string r;
  r.resize(nbytes);
  r.assign(data_, nbytes);
  return r;
}
int BufferReader::readFromSocket(sockfd_t sockfd)
{
  int writeBytes = writableBytes();
  if (writeBytes < g_max_buffer_size) {
    this->reset();
  }

  int len = net::socket_api::recv(sockfd, data_, writeBytes);
  if (len <= 0) return 0;
  return len;
}
std::string BufferReader::readAll()
{
  return this->read(this->readableBytes());
}

int BufferReader::append(std::string_view data)
{
  int writeBytes = this->writableBytes();
  if (writeBytes < g_max_buffer_size) {
    this->reset();
  }

  ::memcpy(peek(), data.data(), data.length());
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
  int len = this->findFirst(matchStr);
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
  int len = ::strlen(target);
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
  memcpy(buffer, data_, capacity_);
  std::swap(buffer, data_);
  delete[] buffer;
  capacity_ *= 2;
}
LY_NAMESPACE_END
