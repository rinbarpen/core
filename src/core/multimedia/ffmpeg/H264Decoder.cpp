#include <core/multimedia/ffmpeg/H264Decoder.h>
#include <core/util/logger/Logger.h>
#include "core/multimedia/ffmpeg/Converter.h"
#include "core/multimedia/ffmpeg/FFmpegUtil.h"
#include "core/util/marcos.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libavcodec/codec_id.h"
#include "libavutil/pixfmt.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)
static auto g_ffmpeg_logger = GET_LOGGER("multimedia.ffmpeg");

H264Decoder::H264Decoder() {
}
H264Decoder::~H264Decoder() {
  this->close();
}

bool H264Decoder::prepare(AVConfig &config) {
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (nullptr == codec) {
		ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Decoder] H264 is not found");
		return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (nullptr == codec_context_) {
		ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Decoder] Out of memory in allocating codec context");
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
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[H264Decoder] Fail to open codec");
		return false;
  }

  config_ = config;
	ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Decoder] Initialize successfully");
	return true;
}
void H264Decoder::close() {
  if (codec_context_) {
    avcodec_free_context(&codec_context_);
    ILOG_DEBUG_FMT(g_ffmpeg_logger, "[H264Decoder] Close successfully");
  }

  pts_ = 0;
}
AVFramePtr H264Decoder::decode(AVPacketPtr pkt) {
  LY_ASSERT(pkt);

  int r = avcodec_send_packet(codec_context_, pkt.get());
  if (r < 0) {
		ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Decoder] Error occurs while sending packet");
    return nullptr;
  }

  AVFramePtr frame = makeFramePtr();
  r = avcodec_receive_frame(codec_context_, frame.get());
  if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
    ILOG_INFO_FMT(g_ffmpeg_logger, "[H264Decoder] No frame to be gotten");
    return nullptr;
  }
  else if (r < 0) {
    ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Decoder] Error occurs while receiving frame");
    return nullptr;
  }

  VideoInfo in, out;
  in.width = frame->width;
  in.height = frame->height;
  in.format = (AVPixelFormat)frame->format;
  out.width = config_.video.width;
  out.height = config_.video.height;
  out.format = config_.video.format;

  converter_.reset(new Converter(in, out));
  AVFramePtr yuv_frame;
  if (converter_->convert(frame, yuv_frame) < 0) {
		ILOG_WARN_FMT(g_ffmpeg_logger, "[H264Decoder] Something bad happened in converting frame");
    return nullptr;
  }

  return yuv_frame;
}

void H264Decoder::setBitRate(uint32_t bit_rate_bps) {
	if (codec_context_) {
		int64_t out_val = 0;
		av_opt_get_int(codec_context_->priv_data, "avcintra-class", 0, &out_val);
		if (out_val < 0) {
			codec_context_->bit_rate = bit_rate_bps;
			codec_context_->rc_min_rate = codec_context_->bit_rate;
			codec_context_->rc_max_rate = codec_context_->bit_rate;
			codec_context_->rc_buffer_size = (int)codec_context_->bit_rate;
			ILOG_INFO_FMT(g_ffmpeg_logger, "[H264Decoder] Set bit rate to a fixed number -- {}", bit_rate_bps);
		}
	}
}

NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END
