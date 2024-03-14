#include <cstdint>
#include <core/util/logger/Logger.h>
#include <core/multimedia/net/AACSource.h>
#include <core/multimedia/net/H264Source.h>
#include <core/multimedia/net/H264Parser.h>
#include <core/multimedia/capture/screen_capture/WASAPIHelper.h>
#include <core/multimedia/capture/screen_capture/GDISrceenCapture.h>
#include <core/multimedia/capture/screen_capture/X11ScreenCapture.h>
#include <core/multimedia/capture/audio_capture/WASAPIAudioCapture.h>
#include <core/multimedia/capture/audio_capture/PulseCapture.h>
#include <ScreenLive.h>

LY_NAMESPACE_BEGIN
static auto g_screen_live_logger = GET_LOGGER("app.screen.live");
using namespace net;

ScreenLive::ScreenLive()
  : event_loop_(new net::EventLoop{}),
    encoding_fps_(0)
{}
ScreenLive::~ScreenLive()
{
  this->destroy();
}

bool ScreenLive::init(ScreenLiveConfig &config) {
  if (initialized_) {
    this->destroy();
  }

  if (!this->startCapture()) {
    return false;
  }
  if (!this->startEncoder(config)) {
    return false;
  }

  initialized_ = true;
  return true;
}

bool ScreenLive::destroy() {
  if (!initialized_) {
    return false;
  }

  {
    Mutex::lock locker(mutex_);

    if (rtsp_pusher_ != nullptr && rtsp_pusher_->isConnected()) {
      rtsp_pusher_->close();
      rtsp_pusher_ = nullptr;
    }
    if (rtmp_pusher_ != nullptr && rtmp_pusher_->isConnected()) {
      rtmp_pusher_->close();
      rtmp_pusher_ = nullptr;
    }
    if (rtsp_server_ != nullptr) {
      rtsp_server_->removeSession(media_session_id_);
      rtsp_server_ = nullptr;
    }
  }

  this->stopEncoder();
  this->stopCapture();
  initialized_ = false;
  return true;
}

bool ScreenLive::startLive(ScreenLiveType type, LiveConfig &config) {
  if (!encoder_started_) {
    return false;
  }

  uint32_t sample_rate = audio_capture_->getSampleRate();
  uint32_t channels = audio_capture_->getChannels();

  if (type == ScreenLiveType::RTSP_SERVER) {
    auto rtsp_server = net::RtspServer::create(event_loop_.get());
    net::MediaSessionId session_id = 0;

    if (config.server.ip == "127.0.0.1") {
      config.server.ip = "0.0.0.0";
    }

    if (!rtsp_server->start(config.server.ip.c_str(), config.server.port, 1000)) {
      return false;
    }

    net::MediaSession *session = net::MediaSession::create(config.server.suffix);
    session->addSource(net::channel_0, net::H264Source::create(screen_live_config_.frame_rate));
    // session->addSource(
    //   net::channel_1, net::AACSource::create(sample_rate, channels, false));
    session->addNotifyConnectedCallback(
      [this](net::MediaSessionId session_id, std::string peer_ip,
        uint16_t peer_port) {
          this->rtsp_clients_.emplace(peer_ip + ":" + std::to_string(peer_port));
        ILOG_INFO(g_screen_live_logger)
          << "RTSP client: " << this->rtsp_clients_.size();
      });
    session->addNotifyDisconnectedCallback(
      [this](net::MediaSessionId session_id, std::string peer_ip,
        uint16_t peer_port) {
          this->rtsp_clients_.erase(peer_ip + ":" + std::to_string(peer_port));
        ILOG_INFO(g_screen_live_logger)
          << "RTSP client: " << this->rtsp_clients_.size();
     });

    session_id = rtsp_server->addSession(session);
    ILOG_INFO_FMT(g_screen_live_logger, "RTSP Server start: tcp://{}:{}",
      config.server.ip, config.server.port);

    Mutex::lock locker(mutex_);
    rtsp_server_ = rtsp_server;
    media_session_id_ = session_id;
    return true;
  }
  else if (type == ScreenLiveType::RTSP_PUSHER) {
    auto rtsp_pusher = net::RtspPusher::create(event_loop_.get());
    auto session = net::MediaSession::create();
    session->addSource(
      net::channel_0, net::H264Source::create(screen_live_config_.frame_rate));
    session->addSource(
      net::channel_1, net::AACSource::create(audio_capture_->getSampleRate(),
                        audio_capture_->getChannels(), false));

    rtsp_pusher->addSession(session);
    if (!rtsp_pusher->openUrl(config.pusher.rtsp_url, 1000ms)) {
      rtsp_pusher = nullptr;
      ILOG_ERROR_FMT(g_screen_live_logger, "RTSP Pusher: Fail to open url({}).",
        config.pusher.rtsp_url);
      return false;
    }

    ILOG_INFO_FMT(g_screen_live_logger, "RTSP Pusher start: Push stream to {} ...",
      config.pusher.rtsp_url);
    Mutex::lock locker(mutex_);
    rtsp_pusher_ = rtsp_pusher;
    return true;
  }
  else if (type == ScreenLiveType::RTMP_PUSHER) {
    auto rtmp_pusher = net::RtmpPublisher::create(event_loop_.get());

    net::MediaInfo mediaInfo;
    uint8_t extradata[1024] = {0};
    int extradata_size = 0;

    extradata_size = aac_encoder_.getSpecificConfig(extradata, 1024);
    if (extradata_size < 0) {
      ILOG_ERROR_FMT(g_screen_live_logger, "Get audio specific config failed.");
      return false;
    }

    mediaInfo.audio_specific_config_size = extradata_size;
    mediaInfo.audio_specific_config.reset(
      new uint8_t[mediaInfo.audio_specific_config_size],
      std::default_delete<uint8_t[]>());
    memcpy(mediaInfo.audio_specific_config.get(), extradata, extradata_size);

    extradata_size = h264_encoder_.getSequenceParams(extradata, 1024);
    if (extradata_size < 0) {
      ILOG_ERROR_FMT(g_screen_live_logger, "Fail to get video specific config.");
      return false;
    }

    net::Nal sps =
      net::H264Parser::findNal((uint8_t *) extradata, extradata_size);
    if (sps.start_pos != nullptr && sps.end_pos != nullptr
        && ((*sps.start_pos & 0x1F) == 7)) {
      mediaInfo.sps_size = sps.end_pos - sps.start_pos + 1;
      mediaInfo.sps.reset(
        new uint8_t[mediaInfo.sps_size], std::default_delete<uint8_t[]>());
      memcpy(mediaInfo.sps.get(), sps.start_pos, mediaInfo.sps_size);

      net::Nal pps = net::H264Parser::findNal(
        sps.end_pos, extradata_size - (sps.end_pos - (uint8_t *) extradata));
      if (pps.start_pos != nullptr && pps.end_pos != nullptr
          && ((*pps.start_pos & 0x1F) == 8)) {
        mediaInfo.pps_size = pps.end_pos - pps.start_pos + 1;
        mediaInfo.pps.reset(
          new uint8_t[mediaInfo.pps_size], std::default_delete<uint8_t[]>());
        memcpy(mediaInfo.pps.get(), pps.start_pos, mediaInfo.pps_size);
      }
    }

    rtmp_pusher->setMediaInfo(mediaInfo);

    std::string status;
    if (!rtmp_pusher->openUrl(config.pusher.rtmp_url, 1000ms, status)) {
      ILOG_ERROR_FMT(g_screen_live_logger,
        "RTMP Pusher: Fail to open url({}). status:{}", config.pusher.rtmp_url, status);
      return false;
    }

    ILOG_INFO_FMT(g_screen_live_logger, "RTMP Pusher start: Push stream to {} ...",
      config.pusher.rtmp_url);
    Mutex::lock locker(mutex_);
    rtmp_pusher_ = rtmp_pusher;
    return true;
  }

  return false;
}

bool ScreenLive::stopLive(ScreenLiveType type) {
  Mutex::lock locker(mutex_);

  switch (type) {
  case ScreenLiveType::RTSP_SERVER:
    if (rtsp_server_ != nullptr) {
      rtsp_server_->stop();
      rtsp_server_ = nullptr;
      rtsp_clients_.clear();
      ILOG_INFO(g_screen_live_logger) << "RTSP Server is stopping.";
    }
    break;
  case ScreenLiveType::RTSP_PUSHER:
    if (rtsp_pusher_ != nullptr) {
      rtsp_pusher_->close();
      rtsp_pusher_ = nullptr;
      ILOG_INFO(g_screen_live_logger) << "RTSP Pusher is stopping.";
    }
    break;
  case ScreenLiveType::RTMP_PUSHER:
    if (rtmp_pusher_ != nullptr) {
      rtmp_pusher_->close();
      rtmp_pusher_ = nullptr;
      ILOG_INFO(g_screen_live_logger) << "RTMP Pusher is stopping.";
    }
    break;
  default: break;
  }

  return true;
}

bool ScreenLive::startCapture()
{
  int display_index = screen_live_config_.display_index;  // monitor index
#if defined(__LINUX__)
  if (!screen_capture_) {
    screen_capture_.reset(new X11ScreenCapture{screen_live_config_.width, screen_live_config_.height});
    if (!screen_capture_->init(display_index)) {
      ILOG_ERROR(g_screen_live_logger) << "Fail to open X11 screen capture, monitor index: " << display_index;
      // (void)screen_capture_.release();
      return false;
    }
    // audio_capture_.reset(new PulseAudioCapture{});
    // if (!audio_capture_->init()) {
    //   ILOG_ERROR(g_screen_live_logger) << "Fail to open Pulse audio capture";
    //   // (void)screen_capture_.release();
    //   // (void)audio_capture_.release();
    //   return false;
    // }
  }
#elif defined(__WIN__)
  auto monitors = DX::monitors();
  if (monitors.empty()) {
    ILOG_ERROR(g_screen_live_logger) << "Monitor not found.";
    return false;
  }

  for (size_t index = 0; index < monitors.size(); index++) {
    ILOG_INFO_FMT(g_screen_live_logger, "Monitor({}) info: {}x{}",
      index,
      monitors[index].right - monitors[index].left,
      monitors[index].bottom - monitors[index].top);
  }

  if (!screen_capture_) {
# if 0
		if (IsWindows8OrGreater()) {
			printf("DXGI Screen capture start, monitor index: %d \n", display_index);
			screen_capture_ = new DXGIScreenCapture();
			if (!screen_capture_->Init(display_index)) {
				printf("DXGI Screen capture start failed, monitor index: %d \n", display_index);
				delete screen_capture_;

				printf("GDI Screen capture start, monitor index: %d \n", display_index);
				screen_capture_ = new GDIScreenCapture();
			}
		}
		else {
			printf("GDI Screen capture start, monitor index: %d \n", display_index);
			screen_capture_ = new GDIScreenCapture();
		}
# else
    ILOG_INFO(g_screen_live_logger) << "GDI Screen capture start, monitor index: " << display_index;
    screen_capture_.reset(new GDIScreenCapture{});
    if (!screen_capture_->init(display_index)) {
      ILOG_ERROR(g_screen_live_logger) << "Screen capture start failed, monitor index: " << display_index;
      screen_capture_.release();
      return false;
    }
  }
# endif
  audio_capture_.reset(new WASAPIAudioCapture{});
  if (!audio_capture_->init()) {
    ILOG_ERROR(g_screen_live_logger) << "Fail to open Pulse audio capture";
    (void)screen_capture_.release();
    (void)audio_capture_.release();
    return false;
  }
#endif

  if (audio_capture_  && !audio_capture_->init()) {
    return false;
  }

  capture_started_ = true;
  return true;
}
bool ScreenLive::stopCapture()
{
  if (capture_started_) {
    if (screen_capture_) {
      screen_capture_->destroy();
      (void)screen_capture_.release();
    }
    if (audio_capture_) {
      audio_capture_->destroy();
      (void)audio_capture_.release();
    }
    capture_started_ = false;
  }

  return true;
}

bool ScreenLive::startEncoder(ScreenLiveConfig &config)
{
  if (!capture_started_) {
    return false;
  }

  screen_live_config_ = config;

  ffmpeg::AVConfig encoder_config;
  encoder_config.video.frame_rate = screen_live_config_.frame_rate;
  encoder_config.video.bit_rate = screen_live_config_.bit_rate_bps;
  encoder_config.video.gop = screen_live_config_.frame_rate;
  encoder_config.video.format = AV_PIX_FMT_BGRA;
  encoder_config.video.width = screen_capture_->getWidth();
  encoder_config.video.height = screen_capture_->getHeight();

  h264_encoder_.setCodec(config.codec);

  if (!h264_encoder_.prepare(screen_live_config_.frame_rate,
        screen_live_config_.bit_rate_bps,
        AV_PIX_FMT_BGRA, screen_capture_->getWidth(),
        screen_capture_->getHeight())) {
    return false;
  }

  int sample_rate = audio_capture_->getSampleRate();
  int channels = audio_capture_->getChannels();
  if (!aac_encoder_.prepare(sample_rate, channels, AV_SAMPLE_FMT_S16, 64000)) {
    return false;
  }

  encoder_started_ = true;
  encode_video_thread_ = std::thread(&ScreenLive::encodeVideo, this);
  encode_audio_thread_ = std::thread(&ScreenLive::encodeAudio, this);
  return true;
}
bool ScreenLive::stopEncoder()
{
  if (encoder_started_) {
    encoder_started_ = false;

    if (encode_video_thread_.joinable()) {
      encode_video_thread_.join();
    }

    if (encode_audio_thread_.joinable()) {
      encode_audio_thread_.join();
    }

    h264_encoder_.close();
    aac_encoder_.close();
  }

  return true;
}

bool ScreenLive::getScreenImage(std::vector<uint8_t>& rgba_image, uint32_t& width, uint32_t& height)
{
  if (screen_capture_) {
    if (screen_capture_->captureFrame(rgba_image, width, height)) {
      return true;
    }
  }
  return false;
}

std::string ScreenLive::getStatusInfo()
{
  std::string info;

  if (encoder_started_) {
    info += "Encoder: " + screen_live_config_.codec + "\r\n";
    info += "Encoding framerate: " +  std::to_string(encoding_fps_) + "\r\n";
  }
  if (rtsp_server_) {
    info += "RTSP Server (connections): " + std::to_string(rtsp_clients_.size()) + "\r\n";
  }
  if (rtsp_pusher_) {
    std::string status =
      rtsp_pusher_->isConnected() ? "connected" : "disconnected";
    info += "RTSP Pusher: " + status + "\r\n";
  }
  if (rtmp_pusher_) {
    std::string status =
      rtmp_pusher_->isConnected() ? "connected" : "disconnected";
    info += "RTMP Pusher: " + status + "\r\n";
  }

  return info;
}

bool ScreenLive::isConnected(ScreenLiveType type) {
  Mutex::lock locker(mutex_);

  bool connected = false;
  switch (type) {
  case ScreenLiveType::RTSP_SERVER:
    if (rtsp_server_ != nullptr) {
      connected = !rtsp_clients_.empty();
    }
    break;

  case ScreenLiveType::RTSP_PUSHER:
    if (rtsp_pusher_ != nullptr) {
      connected = rtsp_pusher_->isConnected();
    }
    break;

  case ScreenLiveType::RTMP_PUSHER:
    if (rtmp_pusher_ != nullptr) {
      connected = rtmp_pusher_->isConnected();
    }
    break;

  default: break;
  }

  return connected;
}

void ScreenLive::encodeVideo()
{
  static Timestamp encoding_ts, update_ts;
  uint32_t encoding_fps = 0;
  uint32_t msec = 1000 / screen_live_config_.frame_rate;
  std::vector<uint8_t> bgra_image;
  std::vector<uint8_t> out_frame;

  while (encoder_started_ && capture_started_) {
    if (update_ts.elapsed().count() >= 1000) {
      update_ts.reset();
      encoding_fps_ = encoding_fps;
      encoding_fps = 0;
    }

    uint32_t delay = msec;
    uint32_t elapsed = encoding_ts.elapsed().count();
    if (elapsed > delay) {
      delay = 0;
    }
    else {
      delay -= elapsed;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    encoding_ts.reset();

    uint32_t timestamp = net::H264Source::getTimestamp();
    int frame_size = 0;
    uint32_t width = 0, height = 0;

    if (screen_capture_->captureFrame(bgra_image, width, height)) {
      if (h264_encoder_.encode(
            &bgra_image[0], width, height, bgra_image.size(), out_frame)
          > 0) {
        if (!out_frame.empty()) {
          encoding_fps += 1;
          this->pushVideo(&out_frame[0], out_frame.size(), timestamp);
        }
      }
    }
  }

  encoding_fps_ = 0;
}
void ScreenLive::encodeAudio()
{
  std::shared_ptr<uint8_t> pcm_buffer(
    new uint8_t[48000 * 8], std::default_delete<uint8_t[]>());
  uint32_t frame_samples = aac_encoder_.getFramesCount();
  uint32_t channel = audio_capture_->getChannels();
  uint32_t sample_rate = audio_capture_->getSampleRate();

  while (encoder_started_) {
    if (audio_capture_->getSamples() >= frame_samples) {
      if (audio_capture_->read(pcm_buffer.get(), frame_samples)
          != frame_samples) {
        continue;
      }

      ffmpeg::AVPacketPtr pkt_ptr =
        aac_encoder_.encode(pcm_buffer.get(), frame_samples);
      if (pkt_ptr) {
        uint32_t timestamp = net::AACSource::getTimestamp(sample_rate);
        this->pushAudio(pkt_ptr->data, pkt_ptr->size, timestamp);
      }
    }
    else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}
void ScreenLive::pushVideo(
  const uint8_t *data, uint32_t size, uint32_t timestamp)
{
  net::SimAVFrame video_frame(size - 4);
  // video_frame.size() = size - 4;
  video_frame.type =
    this->isKeyFrame(data, size) ? net::VIDEO_FRAME_I : net::VIDEO_FRAME_P;
  video_frame.timestamp = timestamp;
  memcpy(video_frame.data.get(), data + 4, size - 4);

  if (size > 0) {
    Mutex::lock locker(mutex_);

    if (rtsp_server_ != nullptr && this->rtsp_clients_.size() > 0) {
      rtsp_server_->pushFrame(media_session_id_, net::channel_0, video_frame);
    }

    if (rtsp_pusher_ != nullptr && rtsp_pusher_->isConnected()) {
      rtsp_pusher_->pushFrame(net::channel_0, video_frame);
    }

    if (rtmp_pusher_ != nullptr && rtmp_pusher_->isConnected()) {
      rtmp_pusher_->pushVideoFrame((uint8_t*)video_frame.data.get(), video_frame.size());
    }
  }
}
void ScreenLive::pushAudio(
  const uint8_t *data, uint32_t size, uint32_t timestamp)
{
  net::SimAVFrame audio_frame(size);
  audio_frame.timestamp = timestamp;
  audio_frame.type = net::AUDIO_FRAME;
  memcpy(audio_frame.data.get(), data, size);

  if (size > 0) {
    Mutex::lock locker(mutex_);

    if (rtsp_server_ != nullptr && this->rtsp_clients_.size() > 0) {
      rtsp_server_->pushFrame(media_session_id_, net::channel_1, audio_frame);
    }

    if (rtsp_pusher_ && rtsp_pusher_->isConnected()) {
      rtsp_pusher_->pushFrame(net::channel_1, audio_frame);
    }

    if (rtmp_pusher_ != nullptr && rtmp_pusher_->isConnected()) {
      rtmp_pusher_->pushAudioFrame((uint8_t*)audio_frame.data.get(), audio_frame.size());
    }
  }
}

bool ScreenLive::isKeyFrame(const uint8_t *data, uint32_t size)
{
  if (size > 4) {
    // 0x67:sps ,0x65:IDR, 0x6: SEI
    if (data[4] == 0x67 || data[4] == 0x65 || data[4] == 0x6
        || data[4] == 0x27) {
      return true;
    }
  }

  return false;
}


LY_NAMESPACE_END
