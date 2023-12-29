#pragma once
#include <cstdint>
#include "ffmpeg_util.h"

namespace ffmpeg
{

struct AudioConfig
{
  uint32_t sample_rate;
  uint32_t bit_rate;
  uint32_t channels;

  AVSampleFormat format;
};

struct VideoConfig
{
  uint32_t width;
  uint32_t height;
  uint32_t frame_rate;
  uint32_t bit_rate;
  uint32_t gop;

  AVPixelFormat format;
};

struct AVConfig
{
  AudioConfig audio;
  VideoConfig video;
};

struct AVEncodeContext
{
  /* Video */
  const uint8_t *yuv;
  uint32_t width;
  uint32_t height;
  uint32_t image_size;
  uint32_t pts{};

  /* Audio */
  const uint8_t *pcm;
  uint32_t samples;
};

class Encoder
{
public:
  Encoder(AVConfig &config) {}
  virtual ~Encoder() = default;

  virtual uint32_t getFrameNum() const { return 0; }
  virtual void useIDR() { }
  virtual void setBitRate(uint32_t bit_rate_kbps) { }

  virtual AVPacketPtr encode(AVEncodeContext ctx) = 0;

  Encoder(const Encoder&) = delete;
  Encoder& operator=(const Encoder&) = delete;

protected:
  AVConfig config_;
  AVCodecContext *codec_context_;
};
}
