#pragma once
#include <core/multimedia/ffmpeg/Converter.h>
#include <core/multimedia/ffmpeg/Encoder.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
class H264Encoder : public Encoder
{
public:
  SHARED_PTR_USING(H264Encoder, ptr);

  H264Encoder();
  ~H264Encoder();

  bool prepare(AVConfig &config) override;
  void close() override;
  /**
   * @brief encode video
   * @param ctx
   * @return
   */
  AVPacketPtr encode(AVEncodeContext ctx) override;
  AVPacketPtr encode(AVFramePtr frame) override;

  void setBitRate(uint32_t bit_rate_bps) override;
  void useIDR() override;

private:
  std::unique_ptr<Converter> converter_;
  int64_t pts_{0};

  uint32_t in_width_ = 0;
  uint32_t in_height_ = 0;

  bool force_idr_{false};
};
}  // namespace ffmpeg
LY_NAMESPACE_END

