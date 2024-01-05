#include "H264Encoder.h"
#include <stdexcept>

namespace ffmpeg
{
H264Encoder::H264Encoder(AVConfig &config) 
	: Encoder(config)
{
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (nullptr == codec)
  { 
		throw std::runtime_error("H264 is not found");
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (nullptr == codec_context_)
  { 
		throw std::runtime_error("Out of memory in allocating H264 context");
  }

  codec_context_->width = config_.video.width;
  codec_context_->height = config_.video.height;
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

  if (codec->id == AV_CODEC_ID_H264)
  {
    av_opt_set(
      codec_context_->priv_data, "preset", "ultrafast", 0);  // ultrafast
  }

  av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);
  av_opt_set_int(codec_context_->priv_data, "forced-idr", 1, 0);
  av_opt_set_int(codec_context_->priv_data, "avcintra-class", -1, 0);

  if (avcodec_open2(codec_context_, codec, NULL) != 0)
  {
    throw std::runtime_error("Fail to open H264 codec");
  }

  in_width_ = config_.video.width;
  in_height_ = config_.video.height;
}
H264Encoder::~H264Encoder() 
{
  if (codec_context_)
  {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
  }
}

AVPacketPtr H264Encoder::encode(AVEncodeContext ctx)
{
	// 格式不对，需要变换
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
		converter_.reset(new Converter(in, out));
	}

	AVFramePtr in_frame = makeFramePtr();
	in_frame->width  = in_width_;
	in_frame->height = in_height_;
	in_frame->format = config_.video.format;
	if (av_frame_get_buffer(in_frame.get(), 32) != 0) {
		return nullptr;
	}

	memcpy(in_frame->data[0], ctx.yuv, ctx.image_size);

	AVFramePtr yuv_frame;
	if (converter_->convert(in_frame, yuv_frame) <= 0) {
		return nullptr;
	}

	// 同步了
	if (pts_ >= 0) {
		yuv_frame->pts = pts_;
	}
	else {
		// 慢了，视频有延迟
		yuv_frame->pts = pts_++;
	}

	yuv_frame->pict_type = AV_PICTURE_TYPE_NONE;
	if (force_idr_) {
		yuv_frame->pict_type = AV_PICTURE_TYPE_I;
		force_idr_ = false;
	}

	if (avcodec_send_frame(codec_context_, yuv_frame.get()) < 0) {
		return nullptr;
	}

	AVPacketPtr packet = makePacketPtr();

	int r = avcodec_receive_packet(codec_context_, packet.get());
	if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
		return nullptr;
	}
	else if (r < 0) {
		return nullptr;
	}

	return packet;
}

void H264Encoder::setBitRate(uint32_t bit_rate_kbps)
{
	if (codec_context_) {
		int64_t out_val = 0;
		av_opt_get_int(codec_context_->priv_data, "avcintra-class", 0, &out_val);
		if (out_val < 0) {
			codec_context_->bit_rate = bit_rate_kbps * 1000;
			codec_context_->rc_min_rate = codec_context_->bit_rate;
			codec_context_->rc_max_rate = codec_context_->bit_rate;
			codec_context_->rc_buffer_size = (int)codec_context_->bit_rate;
		}
	}
}

void H264Encoder::useIDR()
{
  if (codec_context_) {
    force_idr_ = true;
  }
}

}
