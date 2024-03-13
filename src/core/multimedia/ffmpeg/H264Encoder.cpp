#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <core/util/logger/Logger.h>

static auto g_ffmpeg_logger = GET_LOGGER("multimedia.ffmpeg");

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
H264Encoder::H264Encoder()
{}
H264Encoder::~H264Encoder()
{
	this->close();
}
bool H264Encoder::prepare(AVConfig &config)
{
	// const AVCodec *codec = avcodec_find_decoder_by_name("libx264");
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (nullptr == codec) {
		ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Encoder] H264 is not found");
		return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (nullptr == codec_context_) {
		ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Encoder] Out of memory in allocating codec context");
		return false;
  }

  codec_context_->width = config.video.width;
  codec_context_->height = config.video.height;
	// control play speed
  codec_context_->time_base = {1, (int) config.video.frame_rate};
  codec_context_->framerate = {(int) config.video.frame_rate, 1};
  codec_context_->gop_size = config.video.gop;
  codec_context_->max_b_frames = 0;
  codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;

  // rc control mode: abr
  codec_context_->bit_rate = config.video.bit_rate;

  // cbr mode config
  codec_context_->rc_min_rate = config.video.bit_rate;
  codec_context_->rc_max_rate = config.video.bit_rate;
  codec_context_->rc_buffer_size = (int) config.video.bit_rate;

  codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

  if (codec->id == AV_CODEC_ID_H264) {
    av_opt_set(
      codec_context_->priv_data, "preset", "ultrafast", 0);  // ultrafast
  }

  av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);
  av_opt_set_int(codec_context_->priv_data, "forced-idr", 1, 0);
  av_opt_set_int(codec_context_->priv_data, "avcintra-class", -1, 0);

  if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
		avcodec_free_context(&codec_context_);
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Encoder] Fail to open codec");
		return false;
  }

	in_width_ = config.video.width;
  in_height_ = config.video.height;
  config_ = config;
	ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Encoder] Initialize successfully");
	return true;
}
void H264Encoder::close()
{
  if (codec_context_)
  {
    avcodec_free_context(&codec_context_);
		codec_context_ = nullptr;
		ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Encoder] Close successfully");
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
			ILOG_INFO_FMT(g_ffmpeg_logger, "[H264Encoder] Switch on video converter");
		}

		converter_.reset(new Converter(in, out));
	}

	AVFramePtr in_frame = makeFramePtr();
	in_frame->width  = in_width_;
	in_frame->height = in_height_;
	in_frame->format = config_.video.format;
	if (av_frame_get_buffer(in_frame.get(), 32) != 0) {
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Encoder] Out of memory in allocating frame buffer");
    return nullptr;
	}

	memcpy(in_frame->data[0], ctx.yuv, ctx.image_size);

	AVFramePtr yuv_frame;
	if (converter_->convert(in_frame, yuv_frame) <= 0) {
		ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Encoder] Something bad happened in converting frame");
		return nullptr;
	}

	if (ctx.pts >= 0) {
		yuv_frame->pts = ctx.pts;
	}
	else {
		ILOG_TRACE_FMT(g_ffmpeg_logger, "[H264Encoder] Encode slowly, speed up");
		yuv_frame->pts = pts_++;
	}

	yuv_frame->pict_type = AV_PICTURE_TYPE_NONE;
	if (force_idr_) {
		ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Encoder] Set current I frame to IDR frame");
		yuv_frame->pict_type = AV_PICTURE_TYPE_I;
		force_idr_ = false;
	}

	if (avcodec_send_frame(codec_context_, yuv_frame.get()) < 0) {
		ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Encoder] Error occurs while sending frame");
		return nullptr;
	}

	AVPacketPtr packet = makePacketPtr();

	int r = avcodec_receive_packet(codec_context_, packet.get());
	if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
    ILOG_INFO_FMT(g_ffmpeg_logger, "[H264Encoder] No packet to be gotten");
		return nullptr;
	}
	else if (r < 0) {
    ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Encoder] Error occurs while receiving packet");
		return nullptr;
	}

	ILOG_TRACE_FMT(g_ffmpeg_logger, "[H264Encoder] A new packet has arrived");
	return packet;
}

void H264Encoder::setBitRate(uint32_t bit_rate_bps)
{
	if (codec_context_) {
		int64_t out_val = 0;
		av_opt_get_int(codec_context_->priv_data, "avcintra-class", 0, &out_val);
		if (out_val < 0) {
			codec_context_->bit_rate = bit_rate_bps;
			codec_context_->rc_min_rate = codec_context_->bit_rate;
			codec_context_->rc_max_rate = codec_context_->bit_rate;
			codec_context_->rc_buffer_size = (int)codec_context_->bit_rate;
			ILOG_INFO_FMT(g_ffmpeg_logger, "[H264Encoder] Set bit rate to a fixed number -- {}", bit_rate_bps);
		}
	}
}

void H264Encoder::useIDR()
{
  if (codec_context_) {
    force_idr_ = true;
		ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Encoder] Use force idr");
  }
}

}  // namespace ffmpeg
LY_NAMESPACE_END
