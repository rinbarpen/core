#pragma once

#include "core/util/ds/SharedString.h"
#include "core/util/marcos.h"

LY_NAMESPACE_BEGIN
// NOTE: TODO:
struct AVPacket
{
  explicit AVPacket(size_t capacity)
    : data(capacity)
  {}

  SharedString data;
	uint32_t timestamp;
	uint8_t  type{0};
	uint8_t  last;
};

LY_NAMESPACE_END
