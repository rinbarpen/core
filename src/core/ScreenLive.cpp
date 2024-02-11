#include <cstddef>
#include <core/util/logger/Logger.h>
#include <core/net/EventLoop.h>
#include <core/multimedia/net/H264Source.h>
#include <core/multimedia/net/AACSource.h>
#include <core/ScreenLive.h>

#define SCREEN_LIVE_RTSP_SERVER 1
#define SCREEN_LIVE_RTSP_PUSHER 2
#define SCREEN_LIVE_RTMP_PUSHER 3

LY_NAMESPACE_BEGIN

static auto g_screenlive_logger = GET_LOGGER("app.screenlive");

ScreenLive::ScreenLive()
  : event_loop_(new net::EventLoop), encoding_fps_(0)
{
  rtsp_clients_.clear();
}

ScreenLive::~ScreenLive()
{
  this->destroy();
}

auto ScreenLive::instance() -> ScreenLive &
{
  static ScreenLive live;
  return live;
}

bool ScreenLive::getScreenImage(std::vector<uint8_t>& bgra_image, uint32_t& width, uint32_t& height)
{
	if (screen_capture_) {
		if (screen_capture_->captureFrame(bgra_image, width, height)) {
			return true;
		}
	}

	return false;
}

std::string ScreenLive::getStatusInfo() const
{
	std::string info;

	if (encoder_started_) {
		info += "Encoder: " + av_config_.vcodec + " \r\n";
		info += "Encoding framerate: " + std::to_string(encoding_fps_) + " \r\n";
	}

	if (rtsp_server_ != nullptr) {
		info += "RTSP Server (connections): " + std::to_string(rtsp_clients_.size()) + " \r\n";
	}

	if (rtsp_pusher_ != nullptr) {
		std::string status = rtsp_pusher_->isConnected() ? "connected" : "disconnected";
		info += "RTSP Pusher: " + status + " \r\n";
	}

	// if (rtmp_pusher_ != nullptr) {
	// 	std::string status = rtmp_pusher_->isConnected() ? "connected" : "disconnected";
	// 	info += "RTMP Pusher: " + status + " \n\n";
	// }

	return info;
}

bool ScreenLive::init(AVConfig& config)
{
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

void ScreenLive::destroy()
{
	{
		Mutex::lock locker(mutex_);

		if (rtsp_pusher_ && rtsp_pusher_->isConnected()) {
			rtsp_pusher_->close();
			rtsp_pusher_ = nullptr;
		}

		// if (rtmp_pusher_ != nullptr && rtmp_pusher_->isConnected()) {
		// 	rtmp_pusher_->close();
		// 	rtmp_pusher_ = nullptr;
		// }

		if (rtsp_server_) {
			rtsp_server_->removeSession(media_session_id_);
			rtsp_server_ = nullptr;
		}
	}

	this->stopEncoder();
	this->stopCapture();
	initialized_ = false;
}

bool ScreenLive::startLive(int type, LiveConfig& config)
{
	if (!encoder_started_) {
		return false;
	}

	uint32_t samplerate = audio_capture_->getSamplerate();
	uint32_t channels = audio_capture_->getChannels();

	if (type == SCREEN_LIVE_RTSP_SERVER) {
		auto rtsp_server = net::RtspServer::create(event_loop_.get());
		net::MediaSessionId session_id = 0;

		if (config.ip == "127.0.0.1") {
			config.ip = "0.0.0.0";
		}

		if (!rtsp_server->start(config.ip.c_str(), config.port, 10)) {
			return false;
		}

		auto session = net::MediaSession::create(config.suffix);
		session->addSource(net::channel_0, net::H264Source::create(av_config_.framerate));
		session->addSource(net::channel_1, net::AACSource::create(samplerate, channels, false));
		session->addNotifyConnectedCallback([this](net::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port) {
			this->rtsp_clients_.emplace(peer_ip + ":" + std::to_string(peer_port));
      ILOG_INFO_FMT(g_screenlive_logger, "Now there are {} RTSP clients", this->rtsp_clients_.size());
		});
		session->addNotifyDisconnectedCallback([this](net::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port) {
			this->rtsp_clients_.erase(peer_ip + ":" + std::to_string(peer_port));
      ILOG_INFO_FMT(g_screenlive_logger, "Now there are {} RTSP clients", this->rtsp_clients_.size());
		});


		session_id = rtsp_server->addSession(session);
		//printf("RTSP Server: rtsp://%s:%hu/%s \n", net::socket_api::getLocalIPAddress().c_str(), config.port, config.suffix.c_str());
    ILOG_INFO_FMT(g_screenlive_logger, "RTSP Server start: rtsp://{}:{}/{}", config.ip, config.port, config.suffix);

		Mutex::lock locker(mutex_);
		rtsp_server_ = rtsp_server;
		media_session_id_ = session_id;
	}
	else if (type == SCREEN_LIVE_RTSP_PUSHER) {
		auto rtsp_pusher = net::RtspPusher::create(event_loop_.get());
		net::MediaSession *session = net::MediaSession::create();
		session->addSource(net::channel_0, net::H264Source::create(av_config_.framerate));
		session->addSource(net::channel_1, net::AACSource::create(audio_capture_->getSamplerate(), audio_capture_->getChannels(), false));

		rtsp_pusher->addSession(session);
		if (rtsp_pusher->openUrl(config.rtsp_url, 1000ms) != 0) {
			rtsp_pusher = nullptr;
      ILOG_ERROR_FMT(g_screenlive_logger, "RTSP Pusher: Open url({}) failed.", config.rtsp_url);
			return false;
		}

		Mutex::lock locker(mutex_);
		rtsp_pusher_ = rtsp_pusher;
    ILOG_INFO_FMT(g_screenlive_logger, "RTSP Pusher start: Push stream to {} ...", config.rtsp_url);
	}
	// else if (type == SCREEN_LIVE_RTMP_PUSHER) {
	// 	auto rtmp_pusher = net::RtmpPublisher::Create(event_loop_.get());

	// 	net::MediaInfo mediaInfo;
	// 	uint8_t extradata[1024] = { 0 };
	// 	int  extradata_size = 0;

	// 	extradata_size = aac_encoder_.GetSpecificConfig(extradata, 1024);
	// 	if (extradata_size <= 0) {
	// 		printf("Get audio specific config failed. \n");
	// 		return false;
	// 	}

	// 	mediaInfo.audio_specific_config_size = extradata_size;
	// 	mediaInfo.audio_specific_config.reset(new uint8_t[mediaInfo.audio_specific_config_size], std::default_delete<uint8_t[]>());
	// 	memcpy(mediaInfo.audio_specific_config.get(), extradata, extradata_size);

	// 	extradata_size = h264_encoder_.GetSequenceParams(extradata, 1024);
	// 	if (extradata_size <= 0) {
	// 		printf("Get video specific config failed. \n");
	// 		return false;
	// 	}

	// 	net::Nal sps = net::H264Parser::findNal((uint8_t*)extradata, extradata_size);
	// 	if (sps.first != nullptr && sps.second != nullptr && ((*sps.first & 0x1f) == 7)) {
	// 		mediaInfo.sps_size = sps.second - sps.first + 1;
	// 		mediaInfo.sps.reset(new uint8_t[mediaInfo.sps_size], std::default_delete<uint8_t[]>());
	// 		memcpy(mediaInfo.sps.get(), sps.first, mediaInfo.sps_size);

	// 		net::Nal pps = net::H264Parser::findNal(sps.second, extradata_size - (sps.second - (uint8_t*)extradata));
	// 		if (pps.first != nullptr && pps.second != nullptr && ((*pps.first&0x1f) == 8)) {
	// 			mediaInfo.pps_size = pps.second - pps.first + 1;
	// 			mediaInfo.pps.reset(new uint8_t[mediaInfo.pps_size], std::default_delete<uint8_t[]>());
	// 			memcpy(mediaInfo.pps.get(), pps.first, mediaInfo.pps_size);
	// 		}
	// 	}

	// 	rtmp_pusher->SetMediaInfo(mediaInfo);

	// 	std::string status;
	// 	if (rtmp_pusher->OpenUrl(config.rtmp_url, 1000, status) < 0) {
	// 		printf("RTMP Pusher: Open url(%s) failed. \n", config.rtmp_url.c_str());
	// 		return false;
	// 	}

	// 	Mutex::lock locker(mutex_);
	// 	rtmp_pusher_ = rtmp_pusher;
	// 	printf("RTMP Pusher start: Push stream to  %s ... \n", config.rtmp_url.c_str());
	// }
	else {
		return false;
	}

	return true;
}

void ScreenLive::stopLive(int type)
{
	Mutex::lock locker(mutex_);

	switch (type)
	{
	case SCREEN_LIVE_RTSP_SERVER:
		if (rtsp_server_ != nullptr) {
			rtsp_server_->stop();
			rtsp_server_ = nullptr;
			rtsp_clients_.clear();
      ILOG_INFO_FMT(g_screenlive_logger, "RTSP Server stop.");
		}

		break;

	case SCREEN_LIVE_RTSP_PUSHER:
		if (rtsp_pusher_ != nullptr) {
			rtsp_pusher_->close();
			rtsp_pusher_ = nullptr;
      ILOG_INFO_FMT(g_screenlive_logger, "RTSP Pusher stop.");
		}
		break;

	// case SCREEN_LIVE_RTMP_PUSHER:
	// 	if (rtmp_pusher_ != nullptr) {
	// 		rtmp_pusher_->close();
	// 		rtmp_pusher_ = nullptr;
  //    ILOG_INFO_FMT(g_screenlive_logger, "RTMP Pusher stop.");
	// 	}
	// 	break;

	default:
		break;
	}
}

bool ScreenLive::isConnected(int type) const
{
	Mutex::lock locker(mutex_);

	bool is_connected = false;
	switch (type)
	{
	case SCREEN_LIVE_RTSP_SERVER:
		if (rtsp_server_ != nullptr) {
			is_connected = rtsp_clients_.size() > 0;
		}
		break;

	case SCREEN_LIVE_RTSP_PUSHER:
		if (rtsp_pusher_ != nullptr) {
			is_connected = rtsp_pusher_->isConnected();
		}
		break;

	// case SCREEN_LIVE_RTMP_PUSHER:
	// 	if (rtmp_pusher_ != nullptr) {
	// 		is_connected = rtmp_pusher_->isConnected();
	// 	}
	// 	break;

	default:
		break;
	}

	return is_connected;
}

bool ScreenLive::startCapture()
{
	auto monitors = DX::getMonitors();
	if (monitors.empty()) {
    ILOG_ERROR_FMT(g_screenlive_logger, "Monitor not found.");
		return false;
	}

	for (size_t index = 0; index < monitors.size(); index++) {
    ILOG_INFO_FMT(g_screenlive_logger, "Monitor[{}] info: {}x{}",
      index,
      monitors[index].right - monitors[index].left,
			monitors[index].bottom - monitors[index].top);
	}

	int display_index = 0; // monitor index

	if (screen_capture_) {
#if 0
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
#else
		printf("GDI Screen capture start, monitor index: %d \n", display_index);
		screen_capture_ = new GDIScreenCapture();
#endif

		if (!screen_capture_->Init(display_index)) {
			printf("Screen capture start failed, monitor index: %d \n", display_index);
			delete screen_capture_;
			screen_capture_ = nullptr;
			return -1;
		}
	}

	if (!audio_capture_->init()) {
		return false;
	}

	capture_started_ = true;
	return true;
}

int ScreenLive::stopCapture()
{
	if (capture_started_) {
		if (screen_capture_) {
			screen_capture_->destroy();
			screen_capture_ = nullptr;
		}
    if (audio_capture_) {
		  audio_capture_->destroy();
      audio_capture_ = nullptr;
    }
		capture_started_ = false;
	}

	return 0;
}

bool ScreenLive::startEncoder(AVConfig& config)
{
	if (!capture_started_) {
		return false;
	}

	av_config_ = config;

	ffmpeg::AVConfig encoder_config;
	encoder_config.video.frame_rate = av_config_.framerate;
	encoder_config.video.bit_rate = av_config_.bitrate_bps;
	encoder_config.video.gop = av_config_.framerate;
	encoder_config.video.format = AV_PIX_FMT_BGRA;
	encoder_config.video.width = screen_capture_->getWidth();
	encoder_config.video.height = screen_capture_->getHeight();

	h264_encoder_.setCodec(config.vcodec);

	if (!h264_encoder_.init(av_config_.framerate, av_config_.bitrate_bps,
							AV_PIX_FMT_BGRA, screen_capture_->getWidth(),
							screen_capture_->getHeight())) {
		return false;
	}

	int samplerate = audio_capture_.getSamplerate();
	int channels = audio_capture_.getChannels();
	if (!aac_encoder_.init(samplerate, channels, AV_SAMPLE_FMT_S16, 64)) {
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

    if (encode_video_thread_.joinable())
      encode_video_thread_.join();
    if (encode_audio_thread_.joinable())
      encode_audio_thread_.join();

		h264_encoder_.destroy();
		aac_encoder_.destroy();
	}

	return 0;
}

bool ScreenLive::isKeyFrame(const uint8_t* data, uint32_t size)
{
	if (size > 4) {
		//0x67:sps ,0x65:IDR, 0x6: SEI
		if (data[4] == 0x67 || data[4] == 0x65 ||
			data[4] == 0x6 || data[4] == 0x27) {
			return true;
		}
	}

	return false;
}


void ScreenLive::encodeVideo()
{
	static Timestamp encoding_ts, update_ts;
	uint32_t encoding_fps = 0;
	uint32_t msec = 1000 / av_config_.framerate;

	while (encoder_started_ && capture_started_) {
		if (update_ts.elapsed() >= 1000) {
			update_ts.reset();
			encoding_fps_ = encoding_fps;
			encoding_fps = 0;
		}

		uint32_t delay = msec;
		uint32_t elapsed = (uint32_t)encoding_ts.elapsed();
		if (elapsed > delay) {
			delay = 0;
		}
		else {
			delay -= elapsed;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		encoding_ts.reset();

		std::vector<uint8_t> bgra_image;
		uint32_t timestamp = net::H264Source::getTimestamp();
		int frame_size = 0;
		uint32_t width = 0, height = 0;

		if (screen_capture_->captureFrame(bgra_image, width, height)) {
			std::vector<uint8_t> out_frame;
			if (h264_encoder_.encode(bgra_image.data(), width, height, bgra_image.size(), out_frame)) {
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
	std::shared_ptr<uint8_t> pcm_buffer(new uint8_t[48000 * 8], std::default_delete<uint8_t[]>());
	uint32_t frame_samples = aac_encoder_.getFrameNum();
	uint32_t channel = audio_capture_->getChannels();
	uint32_t samplerate = audio_capture_->getSamplerate();

	while (encoder_started_)
	{
		if (audio_capture_->getSamples() >= frame_samples) {
			if (audio_capture_->read(pcm_buffer.get(), frame_samples) != frame_samples) {
				continue;
			}

			ffmpeg::AVPacketPtr pkt_ptr = aac_encoder_.encode(pcm_buffer.get(), frame_samples);
			if (pkt_ptr) {
				uint32_t timestamp = net::AACSource::getTimestamp(samplerate);
				this->pushAudio(pkt_ptr->data, pkt_ptr->size, timestamp);
			}
		}
		else {
			std::this_thread::sleep_for(1ms);
		}
	}
}

LY_NAMESPACE_END
