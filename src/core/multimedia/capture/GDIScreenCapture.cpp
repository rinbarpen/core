#include <core/util/logger/Logger.h>
#include <core/multimedia/capture/GDISrceenCapture.h>
#include <core/multimedia/util/AVClock.h>

LY_NAMESPACE_BEGIN
static auto g_multimedia_capture_logger = GET_LOGGER("multimedia.capture");

GDIScreenCapture::GDIScreenCapture(){}
GDIScreenCapture::~GDIScreenCapture(){}

bool GDIScreenCapture::init(int display_index, bool auto_run)
{
	if (initialized_) {
		return true;
	}

	std::vector<DX::Monitor> monitors = DX::monitors();
	if (monitors.size() < display_index + 1) {
		return false;
	}

	monitor_ = monitors[display_index];

	char video_size[20] = { 0 };
	snprintf(video_size, sizeof(video_size), "%dx%d",
		monitor_.right - monitor_.left, monitor_.bottom - monitor_.top);

	AVDictionary *options = nullptr;
	av_dict_set_int(&options, "framerate", frame_rate_, AV_DICT_MATCH_CASE);
	av_dict_set_int(&options, "draw_mouse", 1, AV_DICT_MATCH_CASE);
	av_dict_set_int(&options, "offset_x", monitor_.left, AV_DICT_MATCH_CASE);
	av_dict_set_int(&options, "offset_y", monitor_.top, AV_DICT_MATCH_CASE);
	av_dict_set(&options, "video_size", video_size, 1);

	input_format_ = av_find_input_format("gdigrab");
	if (nullptr == input_format_) {
		ILOG_ERROR(g_multimedia_capture_logger) << "Gdigrab not found.";
		return false;
	}

	format_context_ = avformat_alloc_context();
	if (avformat_open_input(&format_context_, "desktop", input_format_, &options) != 0) {
		ILOG_ERROR(g_multimedia_capture_logger) << "Open input failed.";
		return false;
	}

	if (avformat_find_stream_info(format_context_, nullptr) < 0) {
		ILOG_ERROR(g_multimedia_capture_logger) << "Couldn't find stream info.";
    avformat_close_input(&format_context_);
		format_context_ = nullptr;
		return false;
	}

	int video_index = -1;
	for (int i = 0; i < format_context_->nb_streams; i++) {
		if (AVMEDIA_TYPE_VIDEO == format_context_->streams[i]->codecpar->codec_type) {
			video_index = i;
      break;
		}
	}

	if (video_index < 0) {
    ILOG_ERROR(g_multimedia_capture_logger) << "Couldn't find video stream.";
		avformat_close_input(&format_context_);
		format_context_ = nullptr;
		return false;
	}

	const AVCodec* codec = avcodec_find_decoder(format_context_->streams[video_index]->codecpar->codec_id);
	if (nullptr == codec) {
    ILOG_ERROR(g_multimedia_capture_logger) << "Couldn't find video codec.";
		avformat_close_input(&format_context_);
		format_context_ = nullptr;
		return false;
	}

	codec_context_ = avcodec_alloc_context3(codec);
	if (nullptr == codec_context_) {
    ILOG_ERROR(g_multimedia_capture_logger) << "Out of memory in allocating codec context";
		return false;
	}

	avcodec_parameters_to_context(codec_context_, format_context_->streams[video_index]->codecpar);
	if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
		avcodec_close(codec_context_);
		codec_context_ = nullptr;
		avformat_close_input(&format_context_);
		format_context_ = nullptr;
		return false;
	}

	video_index_ = video_index;
	initialized_ = true;
	if (auto_run) this->startCapture();
	return true;
}
bool GDIScreenCapture::destroy()
{
	if (!initialized_) {
    return false;
  }

  this->stopCapture();

  if (codec_context_) {
    avcodec_close(codec_context_);
    codec_context_ = nullptr;
  }

  if (format_context_) {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }

  input_format_ = nullptr;
  video_index_ = -1;
  initialized_ = false;
  return true;
}

bool GDIScreenCapture::captureFrame(std::vector<uint8_t>& image, uint32_t& width, uint32_t& height)
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

uint32_t GDIScreenCapture::getWidth() const { return width_; }
uint32_t GDIScreenCapture::getHeight() const { return height_; }
bool GDIScreenCapture::isCapturing() const { return started_; }

bool GDIScreenCapture::startCapture()
{
	if (initialized_ && !started_) {
		started_ = true;
		worker_ = std::thread([this] {
			while (started_) {
				AVClock::sleep(std::chrono::milliseconds(1000 / frame_rate_));
				this->acquireFrame();
			}
		});

		return true;
	}

	return false;
}
void GDIScreenCapture::stopCapture()
{
	if (started_) {
		started_ = false;
		if (worker_.joinable()) worker_.join();

		Mutex::lock locker(mutex_);
		image_.reset();
		width_ = 0;
		height_ = 0;
	}
}
bool GDIScreenCapture::acquireFrame()
{
	if (!started_) {
		return false;
	}

	ffmpeg::AVFramePtr frame;
	ffmpeg::AVPacketPtr packet;

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
bool GDIScreenCapture::decode(ffmpeg::AVFramePtr frame, ffmpeg::AVPacketPtr packet)
{
	int r = avcodec_send_packet(codec_context_, packet.get());
	if (r < 0) {
		return false;
	}

	r = avcodec_receive_frame(codec_context_, frame.get());
	if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
		return true;
	}
	if (r < 0) {
		return false;
	}

	{
    Mutex::lock locker(mutex_);
    image_.resizeAndClear(frame->pkt_size);
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
