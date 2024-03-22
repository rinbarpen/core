#pragma once

#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/Decoder.h>
#include <core/multimedia/ffmpeg/Converter.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)
class H264Decoder : public Decoder {
public:
  H264Decoder();
  ~H264Decoder();

  bool prepare(AVConfig &config) override;
  void close() override;
  AVFramePtr decode(AVPacketPtr pkt) override;

  void setBitRate(uint32_t bit_rate_bps) override;

private:
  std::unique_ptr<Converter> converter_;
  int64_t pts_{0};
};

NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END
