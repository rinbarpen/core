#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <core/multimedia/ffmpeg/Exception.h>
#include <core/util/logger/Logger.h>

static auto g_multimedia_logger = GET_LOGGER("multimedia");

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
H264Encoder::H264Encoder()
{}
H264Encoder::~H264Encoder()
{
	this->close();
}
auto H264Encoder::prepare(AVConfig &config) -> bool
{
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (nullptr == codec) {
		ILOG_ERROR_FMT(g_multimedia_logger, "H264 is not found");
		return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (nullptr == codec_context_) {
		ILOG_ERROR_FMT(g_multimedia_logger, "Out of memory in allocating H264 context");
		return false;
  }

  codec_context_->width = config_.video.width;
  codec_context_->height = config_.video.height;
	// control play speed
  codec_context_->time_base = {1, (int) config_.video.frame_rate};
  codec_context_->framerate = {(int) config_.video.frame_rate, 1};
  codec_context_->gop_size = config_.video.gop;
  codec_context_->max_b_frames = 0;
  codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;

  // rc control mode: abr
  codec_context_->bit_rate = config_.video.bit_rate;

  // cbr mode config
  codec_context_->rc_min_rate = config_.video.bit_rate;
  codec_context_->rc_max_rate = config_.video.bit_rate;
  codec_context_->rc_buffer_size = (int) config_.video.bit_rate;

  codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

  if (codec->id == AV_CODEC_ID_H264) {
    av_opt_set(
      codec_context_->priv_data, "preset", "ultrafast", 0);  // ultrafast
  }

  av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);
  av_opt_set_int(codec_context_->priv_data, "forced-idr", 1, 0);
  av_opt_set_int(codec_context_->priv_data, "avcintra-class", -1, 0);

  if (::avcodec_open2(codec_context_, codec, NULL) != 0)
  {
		::avcodec_free_context(&codec_context_);
    ILOG_ERROR_FMT(g_multimedia_logger, "Fail to open H264 codec");
		return false;
  }

  in_width_ = config_.video.width;
  in_height_ = config_.video.height;
	ILOG_DEBUG_FMT(g_multimedia_logger, "Initalize H264 encoder successfully");
}
void H264Encoder::close()
{
  if (codec_context_)
  {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
		codec_context_ = nullptr;
		ILOG_DEBUG_FMT(g_multimedia_logger, "Close H264 encoder successfully");
	}

	pts_ = 0;
}
AVPacketPtr H264Encoder::encode(AVEncodeContext ctx)
{
	if (ctx.width != in_width_ || ctx.height != in_height_ || !converter_) {
		in_width_ = ctx.width;
		in_height_ = ctx.height;

		VideoInfo in, out;
		in.width  = in_width_;
		in.height = in_height_;
		in.format = config_.video.format;

	  out.width  = codec_context_->width;
	  out.height = codec_context_->height;
		out.format = codec_context_->pix_fmt;

		if (!converter_) {
			ILOG_INFO_FMT(g_multimedia_logger, "Switch on video converter in H264 encoder");
		}

		converter_.reset(new Converter(in, out));
	}

	AVFramePtr in_frame = makeFramePtr();
	in_frame->width  = in_width_;
	in_frame->height = in_height_;
	in_frame->format = config_.video.format;
	if (av_frame_get_buffer(in_frame.get(), 32) != 0) {
    ILOG_ERROR_FMT(g_multimedia_logger, "Out of memory in allocating H264 frame buffer");
    return nullptr;
	}

	memcpy(in_frame->data[0], ctx.yuv, ctx.image_size);

	AVFramePtr yuv_frame;
	if (converter_->convert(in_frame, yuv_frame) <= 0) {
		ILOG_WARN_FMT(g_multimedia_logger, "Something bad happened in converting frame");
		return nullptr;
	}

	yuv_frame->pts = pts_;
	if (pts_ < 0) {
		ILOG_TRACE_FMT(g_multimedia_logger, "H264 is slow, speed up");
		++pts_;
	}

	yuv_frame->pict_type = AV_PICTURE_TYPE_NONE;
	if (force_idr_) {
		ILOG_DEBUG_FMT(g_multimedia_logger, "Set current I frame to IDR frame");
		yuv_frame->pict_type = AV_PICTURE_TYPE_I;
		force_idr_ = false;
	}

	if (avcodec_send_frame(codec_context_, yuv_frame.get()) < 0) {
		ILOG_WARN_FMT(g_multimedia_logger, "Error occurs while sending yuv420p frame from H264 encoder");
		return nullptr;
	}

	AVPacketPtr packet = makePacketPtr();

	int r = avcodec_receive_packet(codec_context_, packet.get());
	if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
    ILOG_INFO_FMT(g_multimedia_logger, "No packet to get in H264 encoder");
		return nullptr;
	}
	else if (r < 0) {
    ILOG_WARN_FMT(g_multimedia_logger, "Error occurs while receiving packet from H264 encoder");
		return nullptr;
	}

	ILOG_TRACE_FMT(g_multimedia_logger, "A new packet has arrived in H264 encoder");
	return packet;
}

void H264Encoder::setBitRate(uint32_t bit_rate_bps)
{
	if (codec_context_) {
		int64_t out_val = 0;
		av_opt_get_int(codec_context_->priv_data, "avcintra-class", 0, &out_val);
		if (out_val < 0) {
			codec_context_->bit_rate = bit_rate_bps;
			// fixed bit rate
			codec_context_->rc_min_rate = codec_context_->bit_rate;
			codec_context_->rc_max_rate = codec_context_->bit_rate;
			// buffer is just enough
			codec_context_->rc_buffer_size = (int)codec_context_->bit_rate;
			ILOG_INFO_FMT(g_multimedia_logger, "Set bit rate to fixed {} in H264 encoder", bit_rate_bps);
		}
	}
}

void H264Encoder::useIDR()
{
  if (codec_context_) {
    force_idr_ = true;
		ILOG_INFO_FMT(g_multimedia_logger, "Use force idr in H264 encoder");
  }
}

}  // namespace ffmpeg
LY_NAMESPACE_END
