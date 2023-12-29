#pragma once

#include <memory>
#include <queue>

#include "core/util/buffer/Buffer.h"

LY_NAMESPACE_BEGIN

void writeU16Forward(char *p, uint16_t value);
void writeU16Reverse(char *p, uint16_t value);
void writeU24Forward(char *p, uint32_t value);
void writeU24Reverse(char *p, uint32_t value);
void writeU32Forward(char *p, uint32_t value);
void writeU32Reverse(char *p, uint32_t value);

class BufferWriter
{
public:
  explicit BufferWriter(size_t qcap) :
    capacity_(qcap)
  {}
  ~BufferWriter() = default;

  bool append(const char* data, uint32_t len);
  int  send(sockfd_t fd, int ms_timeout = 0);

  bool empty() const { return q_.empty(); }
  bool full()  const { return capacity_ == q_.size(); }
  size_t capacity() const { return capacity_; }

  size_t size() const { return q_.size(); }

private:
  std::queue<std::shared_ptr<Buffer>> q_;
  size_t capacity_;
};

LY_NAMESPACE_END
