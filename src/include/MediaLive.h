// play media source file

#include <cstdint>
#include "core/util/marcos.h"

LY_NAMESPACE_BEGIN
struct ScreenLiveConfig
{
  uint32_t bit_rate_bps = 0; // auto
  uint32_t frame_rate = 0;   // auto
  uint32_t gop = 0;          // auto, equal to frame rate

  // [software codec: "x264"]  [hardware codec: "h264_nvenc, h264_qsv"]
  std::string codec = "x264";

  bool operator==(const ScreenLiveConfig& rhs) const {
    return rhs.bit_rate_bps == bit_rate_bps
        && rhs.frame_rate == frame_rate
        && rhs.codec == codec;
  }
  bool operator!=(const ScreenLiveConfig &rhs) const {
    return !(*this == rhs);
  }
};

struct LiveConfig
{
  struct {
    std::string rtsp_url;
    std::string rtmp_url;
  } pusher;
  struct {
    std::string suffix;
    std::string ip;
    uint16_t port;
  } server;
};

enum class ScreenLiveType
{
  RTSP_SERVER,
  RTSP_PUSHER,
  RTMP_PUSHER,
};

class MediaLive
{
public:

private:

};
LY_NAMESPACE_END
