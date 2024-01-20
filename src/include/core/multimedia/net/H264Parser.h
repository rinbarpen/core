#pragma once
#include <cstdint>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
struct Nal
{
  uint8_t *start_pos, *end_pos;
};

class H264Parser
{
public:
  static Nal findNal(const uint8_t *data, uint32_t size);

private:
};
NAMESPACE_END(net)
LY_NAMESPACE_END
