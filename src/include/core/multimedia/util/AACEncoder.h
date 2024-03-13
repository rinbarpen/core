#pragma once
#include <core/multimedia/util/Encoder.h>
#include <core/multimedia/ffmpeg/Resampler.h>

LY_NAMESPACE_BEGIN
#if 0
class AACEncoder final : public Encoder
{
public:
  AACEncoder();
  ~AACEncoder();

  bool open(ffmpeg::AVConfig &config) override;
  bool close() override;
  ffmpeg::AVPacketPtr encode(ffmpeg::AVFramePtr pFrame, EncoderConfig config) override;

  uint32_t getFrameSize() const { return codec_context_->frame_size; }
private:
  ffmpeg::AudioInfo last_audio_info_in_;
  ffmpeg::AudioInfo last_audio_info_out_;
};
#endif
LY_NAMESPACE_END
