#pragma once

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/multimedia/net/rtsp/RtspServer.h>
#include <core/multimedia/net/rtsp/RtspPusher.h>
#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <core/multimedia/ffmpeg/AACEncoder.h>
#include <core/multimedia/capture/ScreenCapture.h>
#include <core/multimedia/capture/AudioCapture.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <set>

LY_NAMESPACE_BEGIN

struct AVConfig
{
  uint32_t bitrate_bps = 800 * 1000;
  uint32_t framerate = 25;
  std::string vcodec = "x264";

  bool operator!=(const AVConfig &rhs) const {
    return bitrate_bps != rhs.bitrate_bps
        || framerate != rhs.framerate
        || vcodec != rhs.vcodec;
  }
};

struct LiveConfig
{
  // pusher
  std::string rtsp_url;
  std::string rtmp_url;

  // server
  std::string ip;
  uint16_t port;
  std::string suffix;
};

class ScreenLive
{
public:
  static auto instance() -> ScreenLive&;

  // ScreenLive();
  ~ScreenLive();

  auto init(AVConfig &config) -> bool;
  void destroy();
  bool isInitialized() const { return initialized_; }

  bool startCapture();
  bool stopCapture();

  bool startEncoder(AVConfig &config);
  bool stopEncoder();
  bool isEncoderInitialized() const { return encoder_started_; }

  bool startLive(int type, LiveConfig& config);
  void stopLive(int type);
  bool isConnected(int type) const;

  bool getScreenImage(std::vector<uint8_t> &bgra_image, uint32_t& width, uint32_t& height);
  std::string getStatusInfo() const;

  LY_NONCOPYABLE(ScreenLive);
private:
	ScreenLive();

	void encodeVideo();
	void encodeAudio();
	void pushVideo(const uint8_t* data, uint32_t size, uint32_t timestamp);
	void pushAudio(const uint8_t* data, uint32_t size, uint32_t timestamp);
	bool isKeyFrame(const uint8_t* data, uint32_t size);


private:
  bool initialized_{false};
  bool capture_started_{false};
  bool encoder_started_{false};

	AVConfig av_config_;
	mutable Mutex::type mutex_;

  // capture
	std::unique_ptr<ScreenCapture> screen_capture_;
	std::unique_ptr<AudioCapture> audio_capture_;

  // encoder
	ffmpeg::H264Encoder h264_encoder_;
	ffmpeg::AACEncoder aac_encoder_;
	std::thread encode_video_thread_;
	std::thread encode_audio_thread_;

	// streamer
	net::MediaSessionId media_session_id_ = 0;
	std::unique_ptr<net::EventLoop> event_loop_ = nullptr;
	std::shared_ptr<net::RtspServer> rtsp_server_ = nullptr;
	std::shared_ptr<net::RtspPusher> rtsp_pusher_ = nullptr;
	// std::shared_ptr<net::RtmpPublisher> rtmp_pusher_ = nullptr;

	// status info
	std::atomic_int encoding_fps_;
	std::set<std::string> rtsp_clients_;
};

LY_NAMESPACE_END
