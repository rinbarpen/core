#include "GDIScreenCapture.h"

#include <functional>

#include "ffmpeg/ffmpeg_util.h"

#include "util/logger/Logger.h"

LY_NAMESPACE_BEGIN
static auto g_av_logger = GET_LOGGER("av");

GDIScreenCapture::GDIScreenCapture(int display_index) 
{
  monitor_ = DX::getMonitor(display_index);
  if (monitor_.low_part == 0)
    throw std::runtime_error("no monitor");

  char video_size[20] = {0};
  snprintf(video_size, sizeof(video_size), "%dx%d",
    monitor_.right - monitor_.left, monitor_.bottom - monitor_.top);

  AVDictionary *options = nullptr;
  av_dict_set_int(&options, "framerate", frame_rate_, AV_DICT_MATCH_CASE);
  av_dict_set_int(&options, "draw_mouse", 1, AV_DICT_MATCH_CASE);
  av_dict_set_int(&options, "offset_x", monitor_.left, AV_DICT_MATCH_CASE);
  av_dict_set_int(&options, "offset_y", monitor_.top, AV_DICT_MATCH_CASE);
  av_dict_set(&options, "video_size", video_size, 1);

  input_format_ = av_find_input_format("gdigrab");
  if (nullptr == input_format_)
  {
    throw std::runtime_error("[GDIScreenCapture] Gdigrab not found.");
  }

  format_context_ = avformat_alloc_context();
  if (avformat_open_input(&format_context_, "desktop", input_format_, &options)
      != 0)
  { 
	throw std::runtime_error("[GDIScreenCapture] Open input failed.");
  }

  if (avformat_find_stream_info(format_context_, nullptr) < 0)
  {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
    throw std::runtime_error("[GDIScreenCapture] Couldn't find stream info.");
  }

  int video_index = -1;
  for (unsigned int i = 0; i < format_context_->nb_streams; i++)
  {
    if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    { video_index = i; }
  }

  if (video_index < 0)
  {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
    throw std::runtime_error("[GDIScreenCapture] Couldn't find video stream.");
  }

  const AVCodec *codec = avcodec_find_decoder(
    format_context_->streams[video_index]->codecpar->codec_id);
  if (!codec)
  {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
    throw std::runtime_error("[GDIScreenCapture] Not support this video codec.");
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (!codec_context_)
  { 
    throw std::runtime_error(
      "[GDIScreenCapture] No space in allocating codec context.");
	}

  avcodec_parameters_to_context(
    codec_context_, format_context_->streams[video_index]->codecpar);
  if (avcodec_open2(codec_context_, codec, nullptr) != 0)
  {
    avcodec_close(codec_context_);
    codec_context_ = nullptr;
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
    throw std::runtime_error(
      "[GDIScreenCapture] Error happens in getting codec context.");
  }

  video_index_ = video_index;
  this->startCapture();
}

GDIScreenCapture::~GDIScreenCapture()
{
  this->stopCapture();

  if (codec_context_)
  {
    avcodec_close(codec_context_);
    codec_context_ = nullptr;
  }

  if (format_context_)
  {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }

  // input_format_ = nullptr;
  // video_index_ = -1;
}

bool GDIScreenCapture::captureFrame(std::vector<uint8_t>& image, uint32_t& width, uint32_t& height)
{
	Mutex::lock locker(mutex_);

	if (!capturing_) {
		image.clear();
		return false;
	}

	if (image_ == nullptr || image_size_ == 0) {
		image.clear();
		return false;
	}

	if (image.capacity() < image_size_) {
		image.reserve(image_size_);
	}

	image.assign(image_.get(), image_.get() + image_size_);
	width = width_;
	height = height_;
	return true;
}

bool GDIScreenCapture::startCapture()
{
  if (capturing_) return false;

  capturing_ = true;	
	thread_ = std::thread([this]() {
	  while(capturing_) {
			Timer::sleep(std::chrono::milliseconds(1000 / frame_rate_));
			this->acquireFrame();
	  }
	});

	return true;
}
void GDIScreenCapture::stopCapture()
{
  if (!capturing_) return;

	LY_ASSERT(thread_.joinable());
	
  thread_.join();
	{
		Mutex::lock locker(mutex_);
		image_.reset();
		image_size_ = 0;
		width_ = 0;
		height_ = 0;
	}
  capturing_ = false;
}
bool GDIScreenCapture::acquireFrame()
{
  LY_ASSERT(capturing_);

	ffmpeg::AVFramePtr pFrame = ffmpeg::makeFramePtr();
	ffmpeg::AVPacketPtr pPacket = ffmpeg::makePacketPtr();

	int ret = av_read_frame(format_context_, pPacket.get());
	if (ret < 0) {
	  return false;
	}

	if (pPacket->stream_index == video_index_) {
	  decode(pFrame, pPacket);
	}

	av_packet_unref(pPacket.get());
	return true;
}
bool GDIScreenCapture::decode(ffmpeg::AVFramePtr pFrame, ffmpeg::AVPacketPtr pPacket)
{
  int ret = avcodec_send_packet(codec_context_, pPacket.get());
	if (ret < 0) {
	  return false;
	}

	ret = avcodec_receive_frame(codec_context_, pFrame.get());
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		return true;
	}

	if (ret < 0) {
		return false;
	}

  {
		Mutex::lock locker(mutex_);

		image_size_ = pFrame->pkt_size;
		image_.reset(new uint8_t[image_size_], std::default_delete<uint8_t[]>());
		width_ = pFrame->width;
		height_ = pFrame->height;

		for (uint32_t i = 0; i < height_; i++) {
			memcpy(image_.get() + i * width_ * 4, pFrame->data[0] + i * pFrame->linesize[0], pFrame->linesize[0]);
		}
  }

	av_frame_unref(pFrame.get());
	return true;
}
LY_NAMESPACE_END
