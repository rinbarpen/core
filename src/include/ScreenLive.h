#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/Singleton.h>
#include <core/net/EventLoop.h>
#include <core/multimedia/net/media.h>
#include <core/multimedia/net/rtsp/RtspPusher.h>
#include <core/multimedia/net/rtsp/RtspServer.h>
#include <core/multimedia/net/rtmp/RtmpPublisher.h>
#include <core/multimedia/codec/encoder/AACEncoder.h>
#include <core/multimedia/codec/encoder/H264Encoder.h>
#include <core/multimedia/capture/audio_capture/AudioCapture.h>
#include <core/multimedia/capture/screen_capture/ScreenCapture.h>


LY_NAMESPACE_BEGIN
struct ScreenLiveConfig
{
  // for video
  uint32_t bit_rate_bps = 800 * 1000;
  uint32_t frame_rate = 25;
  uint32_t gop = 25;

  // [software codec: "h264.ffmpeg"]  [hardware codec: "h264.nvenc, h264.qsv"]
  // h264.ffmpeg, h264.nvenc, h264.qsv
  // h265.ffmpeg
  std::string codec = "h264.ffmpeg";

  uint32_t width = 800, height = 600;  // -1 for auto
  uint32_t offset_x = 0, offset_y = 0;
  int display_index = 0;  // -1 for auto

  bool video_on = true, audio_on = true;
  // for audio
  int volume = 100;
  bool muted = false;

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
class ScreenLive
{
public:
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
  AACEncoder aac_encoder_;
  std::thread encode_video_thread_;
  std::thread encode_audio_thread_;

  // streamer
  net::MediaSessionId media_session_id_{0};
  std::unique_ptr<net::EventLoop> event_loop_;
  std::shared_ptr<net::RtspServer> rtsp_server_;
  std::shared_ptr<net::RtspPusher> rtsp_pusher_;
  std::shared_ptr<net::RtmpPublisher> rtmp_pusher_;

  // status info
  std::atomic_int encoding_fps_;
  std::set<std::string> rtsp_clients_;
};

using SingleScreenLive = Singleton<ScreenLive>;
LY_NAMESPACE_END
