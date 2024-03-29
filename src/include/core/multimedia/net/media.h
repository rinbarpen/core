#pragma once

#include <core/util/ds/SharedString.h>
#include <cstdint>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
enum MediaType
{
  NONE = 0,
  PCMA = 8,
  H264 = 96,
  AAC  = 37,
  H265 = 265,
};

enum FrameType
{
  VIDEO_FRAME_I = 0x01,
  VIDEO_FRAME_P = 0x02,
  VIDEO_FRAME_B = 0x03,
  AUDIO_FRAME   = 0x11,
};

struct SimAVFrame
{
  SimAVFrame(uint32_t size = 0) :
    data(size),
    type(0), timestamp(0)
  {}

  size_t size() const { return data.size(); }

  SharedString<uint8_t> data;
  uint8_t type;
  uint32_t timestamp;
};

static constexpr int kMaxMediaChannel = 2;

enum MediaChannelId
{
  channel_0,
  channel_1,
};

using MediaSessionId = uint32_t;
NAMESPACE_END(net)
LY_NAMESPACE_END
