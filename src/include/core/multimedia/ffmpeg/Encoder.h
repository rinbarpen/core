#pragma once

#include <cstdint>
#include <core/util/marcos.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)
class Encoder
{
public:
  Encoder() = default;
  virtual ~Encoder() = default;

  virtual bool prepare(AVConfig &config) = 0;
  virtual void close() = 0;
  virtual AVPacketPtr encode(AVEncodeContext ctx) { LY_NOT_IMPLEMENT(); return nullptr; }
  virtual AVPacketPtr encode(AVFramePtr frame) { LY_NOT_IMPLEMENT(); return nullptr; }

  virtual uint32_t getFrameNum() const { return 0; }
  virtual void useIDR() { }
  virtual void setBitRate(uint32_t bit_rate_bps) { }

  AVCodecContext *getAVCodecContext() const { return codec_context_; }

  LY_NONCOPYABLE(Encoder);

protected:
  AVConfig config_;
  AVCodecContext *codec_context_{nullptr};
};
NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END
