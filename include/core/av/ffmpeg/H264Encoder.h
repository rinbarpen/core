#pragma once
#include "Converter.h"
#include "Encoder.h"

namespace ffmpeg
{
class H264Encoder : public Encoder
{
public:
  H264Encoder(AVConfig &config);
  ~H264Encoder();

  /**
   * \brief encode video 
   * \param ctx 
   * \return 
   */
  AVPacketPtr encode(AVEncodeContext ctx) override;

  void setBitRate(uint32_t bit_rate_kbps) override;
  void useIDR() override;

private:
  std::unique_ptr<Converter> converter_;
  int64_t pts_{0};

  uint32_t in_width_ = 0;
  uint32_t in_height_ = 0;

  bool force_idr_{false};
};


}

