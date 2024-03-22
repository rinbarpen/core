#include <core/config/config.h>
#include <core/util/logger/Logger.h>
#include <core/multimedia/capture/audio_capture/AlsaCapture.h>

LY_NAMESPACE_BEGIN
static auto g_ffmpeg_debug = LY_CONFIG_GET(multimedia.ffmpeg.open_debug);
static auto g_capture_logger = GET_LOGGER("multimedia.capture");

AlsaCapture::AlsaCapture()
  : pcm_buffer_(48000 * 32 * 2 * 2)
{}
AlsaCapture::~AlsaCapture() {}

bool AlsaCapture::init()
{
  int r{-1};
#if 0
  // open a pcm device
  r = snd_pcm_open(&handle_, "hw:0", SND_PCM_STREAM_CAPTURE, 0);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Unable to open pcm device: "
      << snd_strerror(r);
    return false;
  }
  snd_pcm_hw_params_t *params;
  // allocate memory
  r = snd_pcm_hw_params_malloc(&params);
  // initialize pcm device
  r = snd_pcm_hw_params_any(handle_, params);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Can not configure this PCM device: "
      << snd_strerror(r);
    return false;
  }
  r = snd_pcm_hw_params_set_access(handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to interleaved: "
      << snd_strerror(r);
    return false;
  }
  r = snd_pcm_hw_params_set_format(handle_, params, SND_PCM_FORMAT_S16_LE);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to 16-bit signed PCM: "
      << snd_strerror(r);
    return false;
  }
  r = snd_pcm_hw_params_set_channels(handle_, params, kChannels);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to mono: "
      << snd_strerror(r);
    return false;
  }
  uint32_t val = 48000;
  int dir;
  r = snd_pcm_hw_params_set_rate_near(handle_, params, &val, &dir);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to sample rate = "
      << val
      << ": "
      << snd_strerror(r);
    return false;
  }
  uint32_t buffer_time, period_time;
  snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, nullptr);
  if (buffer_time > 500000) buffer_time = 500000;
  r = snd_pcm_hw_params_set_buffer_time_near(handle_, params, &val, nullptr);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to buffer time = "
      << buffer_time
      << ": "
      << snd_strerror(r);
    return false;
  }
  period_time = 26315;
  // 设置周期时间
  r = snd_pcm_hw_params_set_period_time_near(handle_, params, &period_time, 0);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Failed to set PCM device to period time = "
      << period_time
      << ": "
      << snd_strerror(r);
    return false;
  }
  r = snd_pcm_hw_params(handle_, params);
  if (r < 0) {
    ILOG_ERROR(g_capture_logger)
      << "Unable to set hw parameters: "
      << snd_strerror(r);
    return false;
  }

  snd_pcm_uframes_t frames;
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  uint32_t size = frames * kChannels;
  char *buffer = new char[size];
#endif
  // input_format_ = av_find_input_format("alsa");
	// if (nullptr == input_format_) {
	// 	ILOG_ERROR(g_capture_logger) << "Alsa not found.";
	// 	return false;
	// }
  // format_context_ = avformat_alloc_context();

  // if (avformat_open_input(&format_context_, "hw:0,0", input_format_, nullptr) < 0) {
	// 	ILOG_ERROR(g_capture_logger) << "Fail to open alsa.";
  //   avformat_close_input(&format_context_);
  //   return false;
  // }

	// if (avformat_find_stream_info(format_context_, nullptr) < 0) {
	// 	ILOG_ERROR(g_capture_logger) << "Couldn't find stream info.";
  //   avformat_close_input(&format_context_);
	// 	return false;
	// }

  // AVCodec *codec;
  // int stream_index = av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  // if (stream_index < 0) {
  //   ILOG_ERROR(g_capture_logger) << "No source";
	// 	avformat_close_input(&format_context_);
	// 	return false;
  // }
  // if (g_ffmpeg_debug)
  //   av_dump_format(format_context_, stream_index, "hw:0,0", 0);

	// codec_context_ = avcodec_alloc_context3(codec);
	// if (nullptr == codec_context_) {
  //   ILOG_ERROR(g_capture_logger) << "Out of memory in allocating codec context";
	// 	avformat_close_input(&format_context_);
	// 	return false;
	// }

	// avcodec_parameters_to_context(codec_context_, format_context_->streams[stream_index]->codecpar);
	// if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
	// 	avcodec_close(codec_context_);
	// 	avformat_close_input(&format_context_);
	// 	return false;
	// }

  // stream_index_ = stream_index;
  initialized_ = true;
  return true;
}
bool AlsaCapture::destroy()
{
	if (!initialized_) {
    return false;
  }
  // if (codec_context_) {
  //   avcodec_close(codec_context_);
  // }
  // if (format_context_) {
  //   avformat_close_input(&format_context_);
  // }
#if 0
  if (handle_) {
    snd_pcm_drain(handle_);
    snd_pcm_close(handle_);
  }
#endif

  // input_format_ = nullptr;
  // stream_index_ = -1;
  initialized_ = false;
  return true;
}

bool AlsaCapture::start()
{
#ifdef __LINUX__
  if (enabled_) { return true; }

  enabled_ = true;
  thread_.dispatch([this](){
    while (this->enabled_) {
      if (!this->capture()) {
        break;
      }
    }
  });

  return true;
#else
  return false;
#endif
}
bool AlsaCapture::stop()
{
#ifdef __LINUX__
  Mutex::lock locker(mutex_);
  if (enabled_) {
    enabled_ = false;
    thread_.stop();
  }

  return true;
#else
  return false;
#endif
}

void AlsaCapture::setCallback(PacketCallback callback) {
  Mutex::lock locker(mutex2_);
  callback_ = callback;
}

AudioFormat AlsaCapture::getAudioFormat() const
{
#ifdef __LINUX__
  AudioFormat format;
  format.channels = kChannels;
  format.bits_per_sample = kSampleRate;
  format.samples_per_sec = 1024;
  return format;
#else
  return {};
#endif
}

bool AlsaCapture::capture()
{
#ifdef __LINUX__
  auto frame = ffmpeg::makeFramePtr();
  avcodec_receive_frame(codec_context_, frame.get());

  pcm_buffer_.fill((uint8_t*)frame.get(), sizeof(*frame.get()));
#endif
  return true;
}
LY_NAMESPACE_END
