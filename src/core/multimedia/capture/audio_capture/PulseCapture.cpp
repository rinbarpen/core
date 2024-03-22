#include <core/multimedia/capture/audio_capture/PulseCapture.h>
#include <core/config/config.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
static auto g_ffmpeg_debug = LY_CONFIG_GET(multimedia.ffmpeg.open_debug);
static auto g_capture_logger = GET_LOGGER("multimedia.capture");

PulseCapture::PulseCapture()
  : pcm_buffer_(48000 * 32 * 2 * 2)
{}
PulseCapture::~PulseCapture() {}

bool PulseCapture::init()
{
  input_format_ = av_find_input_format("pulse");
	if (nullptr == input_format_) {
		ILOG_ERROR(g_capture_logger) << "Pulse not found.";
		return false;
	}
  format_context_ = avformat_alloc_context();

  AVDictionary *options;
  av_dict_set_int(&options, "ar", 48000, AV_DICT_MATCH_CASE);
  av_dict_set_int(&options, "ac", 2, AV_DICT_MATCH_CASE);

  if (avformat_open_input(&format_context_, "hw:0,0", input_format_, &options) < 0) {
		ILOG_ERROR(g_capture_logger) << "Fail to open pulse.";
    avformat_close_input(&format_context_);
    return false;
  }

	if (avformat_find_stream_info(format_context_, nullptr) < 0) {
		ILOG_ERROR(g_capture_logger) << "Couldn't find stream info.";
    avformat_close_input(&format_context_);
		return false;
	}

  AVCodec *codec;
  int stream_index = av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  if (stream_index < 0) {
    ILOG_ERROR(g_capture_logger) << "No source";
		avformat_close_input(&format_context_);
		return false;
  }
  if (g_ffmpeg_debug)
    av_dump_format(format_context_, stream_index, "hw:0,0", 0);

	codec_context_ = avcodec_alloc_context3(codec);
	if (nullptr == codec_context_) {
    ILOG_ERROR(g_capture_logger) << "Out of memory in allocating codec context";
		avformat_close_input(&format_context_);
		return false;
	}

	avcodec_parameters_to_context(codec_context_, format_context_->streams[stream_index]->codecpar);
	if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
		avcodec_close(codec_context_);
		avformat_close_input(&format_context_);
		return false;
	}

  stream_index_ = stream_index;
  initialized_ = true;
  return true;
err:
  if (format_context_) {
    avformat_close_input(&format_context_);
    avformat_free_context(format_context_);
  }
  return false;
}
bool PulseCapture::destroy()
{
	if (!initialized_) {
    return false;
  }
  if (codec_context_) {
    avcodec_close(codec_context_);
  }
  if (format_context_) {
    avformat_close_input(&format_context_);
  }

  input_format_ = nullptr;
  stream_index_ = -1;
  initialized_ = false;
  return true;
}

bool PulseCapture::start()
{
  return false;
}
bool PulseCapture::stop()
{
  return true;
}

AudioFormat PulseCapture::getAudioFormat() const
{
#ifdef __LINUX__
  AudioFormat format;
  format.channels = 2;
  format.bits_per_sample = 48000;
  format.samples_per_sec = 1024;
  return format;
#else
  return {};
#endif
}

bool PulseCapture::capture()
{
  return true;
}
LY_NAMESPACE_END
