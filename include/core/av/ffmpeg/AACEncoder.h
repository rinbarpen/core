#pragma once
#include "Encoder.h"
#include "Resampler.h"

namespace ffmpeg
{
class AACEncoder : public Encoder
{
public:
  AACEncoder(AVConfig &config);
  ~AACEncoder() {}

  /**
   * \brief convert pcm to frame,
   *        and convert frame to packet
   * \param ctx
   * \return 
   */
  AVPacketPtr encode(AVEncodeContext ctx) override;

  uint32_t getFrameNum() const override;
private:
  std::unique_ptr<Resampler> resampler_;
  int64_t pts_{0};
};

}
