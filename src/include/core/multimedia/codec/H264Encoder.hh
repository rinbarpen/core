#include "core/util/marcos.h"
#include <core/multimedia/ffmpeg/H264Encoder.h>

LY_NAMESPACE_BEGIN
enum CodecId
{
  H264_NV12,
  H264_DXD,
  H264_FFMPEG,
};
/**
 * @brief use nv12 first, second is d3dx, third is ffmpeg
 *
 */
class H264Encoder
{
public:
  H264Encoder(CodecId h264CodecId);
  H264Encoder(std::string_view h264CodecName);
  ~H264Encoder() = default;

  // auto prepare(EncodeConfig &config) -> bool;
  // void close();
  // /**
  //  * @brief encode video
  //  * @param ctx
  //  * @return
  //  */
  // auto encode(EncodeContext ctx);

  void setBitRate(uint32_t bit_rate_bps);
  void useIDR();

private:
  // bool use_ffmpeg_{false};
  // bool use_nv_{false};
  // bool use_dxd_{false};

  ffmpeg::H264Encoder::ptr ffmpeg_encoder_;

};
LY_NAMESPACE_END
