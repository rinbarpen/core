#include <core/util/logger/Logger.h>
#include <core/multimedia/util/AVClock.h>
#include <core/multimedia/capture/screen_capture/X11ScreenCapture.h>
#include <libavcodec/avcodec.h>
#include "core/config/config.h"
#include "libavformat/avformat.h"

#ifdef __LINUX__
# include <X11/X.h>
# include <X11/Xlib.h>
#endif

LY_NAMESPACE_BEGIN
static auto g_capture_logger = GET_LOGGER("multimedia.capture");
static auto g_ffmpeg_debug_on =  LY_CONFIG_GET(multimedia.ffmpeg.open_debug);

X11ScreenCapture::X11ScreenCapture(uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y)
	: width_(width), height_(height), offset_x_(offset_x), offset_y_(offset_y)
{}
X11ScreenCapture::~X11ScreenCapture(){}

bool X11ScreenCapture::init(int display_index, bool auto_run)
{
	if (initialized_) {
		return true;
	}

	char display_screen[20] = { 0 };
	snprintf(display_screen, 20, ":0.%d+%d+%d", display_index, offset_x_, offset_y_);
	char video_size[20] = { 0 };
	snprintf(video_size, sizeof(video_size), "%dx%d",
		width_, height_);

	AVDictionary *options = nullptr;
	av_dict_set_int(&options, "framerate", frame_rate_, AV_DICT_MATCH_CASE);
	av_dict_set_int(&options, "draw_mouse", 1, AV_DICT_MATCH_CASE);
	// av_dict_set_int(&options, "offset_x", offset_x_, AV_DICT_MATCH_CASE);
	// av_dict_set_int(&options, "offset_y", offset_y_, AV_DICT_MATCH_CASE);
	av_dict_set(&options, "video_size", video_size, AV_DICT_MATCH_CASE);

	input_format_ = av_find_input_format("x11grab");
	if (nullptr == input_format_) {
		ILOG_ERROR(g_capture_logger) << "X11grab not found.";
		return false;
	}

	format_context_ = avformat_alloc_context();
	if (avformat_open_input(&format_context_, display_screen, input_format_, &options) != 0) {
		ILOG_ERROR(g_capture_logger) << "Could not open input device: " << display_screen;
		return false;
	}

	if (avformat_find_stream_info(format_context_, nullptr) < 0) {
		ILOG_ERROR(g_capture_logger) << "Couldn't find stream info.";
		avformat_close_input(&format_context_);
		return false;
	}

	// AVCodec *codec_ = nullptr;
	// video_index = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, &codec_, 0);
	int video_index = -1;
	for (int i = 0; i < format_context_->nb_streams; i++) {
		if (AVMEDIA_TYPE_VIDEO == format_context_->streams[i]->codecpar->codec_type) {
			video_index = i;
      break;
		}
	}
	if (g_ffmpeg_debug_on) {
		av_dump_format(format_context_, video_index, display_screen, 0);
	}

	if (video_index < 0) {
    ILOG_ERROR(g_capture_logger) << "Couldn't find video stream.";
		avformat_close_input(&format_context_);
		return false;
	}

	const AVCodec* codec = avcodec_find_decoder(format_context_->streams[video_index]->codecpar->codec_id);
	if (nullptr == codec) {
    ILOG_ERROR(g_capture_logger) << "Couldn't find video codec.";
		avformat_close_input(&format_context_);
		return false;
	}

	codec_context_ = avcodec_alloc_context3(codec);
	if (nullptr == codec_context_) {
    ILOG_ERROR(g_capture_logger) << "Out of memory in allocating codec context";
		avformat_close_input(&format_context_);
		return false;
	}

	avcodec_parameters_to_context(codec_context_, format_context_->streams[video_index]->codecpar);
	if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
		avcodec_free_context(&codec_context_);
		avformat_close_input(&format_context_);
		return false;
	}

	video_index_ = video_index;
	initialized_ = true;
	if (auto_run) this->startCapture();
	return true;
}
bool X11ScreenCapture::destroy()
{
	if (!initialized_) {
    return false;
  }

  this->stopCapture();

  if (codec_context_) {
    avcodec_free_context(&codec_context_);
  }
  if (format_context_) {
    avformat_close_input(&format_context_);
  }

  input_format_ = nullptr;
  video_index_ = -1;
  initialized_ = false;
  return true;
}

bool X11ScreenCapture::captureFrame(std::vector<uint8_t>& image, uint32_t& width, uint32_t& height)
{
	Mutex::lock locker(mutex_);
	if (!started_) {
		image.clear();
		return false;
	}

	if (image_.get() == nullptr || image_.empty()) {
		image.clear();
		return false;
	}
	if (image.capacity() < image_.size()) {
		image.reserve(image_.size());
	}

	image.assign(image_.get(), image_.get() + image_.size());
	width = width_;
	height = height_;
	return true;
}

uint32_t X11ScreenCapture::getWidth() const { return width_; }
uint32_t X11ScreenCapture::getHeight() const { return height_; }
bool X11ScreenCapture::isCapturing() const { return started_; }

bool X11ScreenCapture::startCapture()
{
	if (initialized_ && !started_) {
		started_ = true;
		worker_.dispatch([this] {
			while (started_) {
				AVClock::sleepMS(1000 / frame_rate_);
				this->acquireFrame();
			}
		});

		return true;
	}

	return false;
}
void X11ScreenCapture::stopCapture()
{
	if (started_) {
		started_ = false;
		worker_.destroy();

		Mutex::lock locker(mutex_);
		image_.reset();
		width_ = 0;
		height_ = 0;
	}
}
bool X11ScreenCapture::acquireFrame()
{
	if (!started_) {
		return false;
	}

	ffmpeg::AVFramePtr frame = ffmpeg::makeFramePtr();
	ffmpeg::AVPacketPtr packet = ffmpeg::makePacketPtr();

	int r = av_read_frame(format_context_, packet.get());
	if (r < 0) {
		return false;
	}

	if (packet->stream_index == video_index_) {
		this->decode(frame, packet);
	}

	av_packet_unref(packet.get());
	return true;
}
bool X11ScreenCapture::decode(ffmpeg::AVFramePtr frame, ffmpeg::AVPacketPtr packet)
{
	int r = avcodec_send_packet(codec_context_, packet.get());
	if (r < 0) {
		return false;
	}

	r = avcodec_receive_frame(codec_context_, frame.get());
	if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
		return true;
	}
	else if (r < 0) {
		return false;
	}

	{
    Mutex::lock locker(mutex_);
    image_.reset(frame->pkt_size);
    width_ = frame->width;
    height_ = frame->height;

    for (uint32_t i = 0; i < height_; i++) {
      memcpy(image_.get() + i * width_ * 4,
        frame->data[0] + i * frame->linesize[0], frame->linesize[0]);
    }
	}

	av_frame_unref(frame.get());
	return true;
}
LY_NAMESPACE_END
