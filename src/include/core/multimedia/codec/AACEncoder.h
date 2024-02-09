#pragma once

#include "core/util/marcos.h"
#include <core/multimedia/ffmpeg/AACEncoder.h>
#include <cstdint>

LY_NAMESPACE_BEGIN
class AACEncoder
{
public:
  AACEncoder() = default;
  ~AACEncoder() = default;

  bool init(int sample_rate, int channels, int format, int bit_rate_bps);
  bool destroy();

  int getFramesCount() const;
  int getSampleRate() const;
  int getChannelsCount() const;
  int getSpecificConfig(uint8_t *buf, int max_buf_size) const;

  ffmpeg::AVPacketPtr encode(const uint8_t *pcm, int samples);

private:
  ffmpeg::AACEncoder aac_encoder_;
  int sample_rate_;
  int channels_;
  int bit_rate_;
  AVSampleFormat format_;
};

LY_NAMESPACE_END
