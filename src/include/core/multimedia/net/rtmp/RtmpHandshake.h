#pragma once
#include <core/util/marcos.h>
#include "core/util/buffer/BufferReader.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpHandshake
{
public:
  enum Status
  {
    HANDSHAKE_C0C1,
    HANDSHAKE_S0S1S2,
    HANDSHAKE_C2,
    HANDSHAKE_COMPLETE,
  };

  RtmpHandshake(Status status);
  virtual ~RtmpHandshake();

  int parse(BufferReader &in, char *res_buf, uint32_t res_buf_size);
  bool buildC0C1(char *buf, uint32_t buf_size);

  bool isCompleted() const { return handshake_status_ == HANDSHAKE_COMPLETE; }

private:
  Status handshake_status_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
