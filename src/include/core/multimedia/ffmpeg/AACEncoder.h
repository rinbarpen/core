#pragma once
#include <core/multimedia/ffmpeg/Encoder.h>
#include <core/multimedia/ffmpeg/Resampler.h>

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
class AACEncoder : public Encoder
{
public:
  AACEncoder();
  ~AACEncoder();

  bool prepare(AVConfig &config) override;
  void close() override;
  /**
   * \brief convert pcm to frame,
   *        and convert frame to packet
   * \param ctx
   * \return
   */
  AVPacketPtr encode(AVEncodeContext ctx) override;

  uint32_t getFrameSize() const { return codec_context_->frame_size; }

private:
  std::unique_ptr<Resampler> resampler_;
  int64_t pts_{0};
};
}
LY_NAMESPACE_END
