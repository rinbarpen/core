#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "core/util/marcos.h"
#include "core/util/Mutex.h"
#include "core/util/Singleton.h"
#include "core/multimedia/capture/AudioCapture.h"
#include "core/multimedia/capture/ScreenCapture.h"
#include "core/multimedia/codec/AACEncoder.h"
#include "core/multimedia/codec/H264Encoder.h"
#include "core/multimedia/net/media.h"
#include "core/multimedia/net/rtsp/RtspPusher.h"
#include "core/multimedia/net/rtsp/RtspServer.h"
#include "core/net/EventLoop.h"


LY_NAMESPACE_BEGIN
struct ScreenLiveConfig
{
  uint32_t bit_rate_bps = 800 * 1000;
  uint32_t frame_rate = 25;
  uint32_t gop = 25;

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

class ScreenLive
{
public:
  enum class ScreenLiveType
  {
    RTSP_SERVER,
    RTSP_PUSHER,
    RTMP_PUSHER,
  };

  ScreenLive();
  ~ScreenLive();

  bool init(ScreenLiveConfig &config);
  bool destroy();

  bool startCapture();
  bool stopCapture();

  bool startEncoder(ScreenLiveConfig &config);
  bool stopEncoder();

  bool startLive(ScreenLiveType type, LiveConfig &config);
  bool stopLive(ScreenLiveType type);

  bool getScreenImage(std::vector<uint8_t> &rgba_image, uint32_t &width, uint32_t &height);
  std::string getStatusInfo();

  bool isInitialized() const { return initialized_; }
  bool isEncoderInitialized() const { return encoder_started_; }
  bool isConnected(ScreenLiveType type); 

  LY_NONCOPYABLE(ScreenLive);
private:
  void encodeVideo();
  void encodeAudio();
  void pushVideo(const uint8_t *data, uint32_t size, uint32_t timestamp);
  void pushAudio(const uint8_t *data, uint32_t size, uint32_t timestamp);

  bool isKeyFrame(const uint8_t *data, uint32_t size);

private:
  bool initialized_{false};
  bool capture_started_{false};
  bool encoder_started_{false};

  ScreenLiveConfig screen_live_config_;
  Mutex::type mutex_;

  // capture
  std::unique_ptr<ScreenCapture> screen_capture_;
  std::unique_ptr<AudioCapture> audio_capture_;

  // encoder
  H264Encoder h264_encoder_;
  AACEncoder acc_encoder_;
  std::thread encode_video_thread_;
  std::thread encode_audio_thread_;

  // streamer
  MediaSessionId media_session_id_{0};
  std::unique_ptr<net::EventLoop> event_loop_;
  std::shared_ptr<RtspServer> rtsp_server_;
  std::shared_ptr<RtspPusher> rtsp_pusher_;
  std::shared_ptr<RtmpPublisher> rtmp_pusher_;

  // status info
  std::atomic_int encoding_fps_;
  std::set<std::string> rtsp_clients_;
};

using SingleScreenLive = Singleton<ScreenLive>;
LY_NAMESPACE_END
