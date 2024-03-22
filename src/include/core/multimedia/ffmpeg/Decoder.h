#pragma once

#include <core/util/marcos.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)
class Decoder
{
public:
  Decoder() = default;
  virtual ~Decoder() = default;

  bool prepare(AVCodecContext *codec_context) {
    if (!codec_context || !av_codec_is_decoder(codec_context->codec)) {
      return false;
    }
    config_.video.width = codec_context->width;
    config_.video.height = codec_context->height;
    config_.video.format = codec_context->pix_fmt;
    config_.video.frame_rate = codec_context->frame_size;
    config_.video.gop = codec_context->gop_size;
    config_.video.bit_rate = codec_context->bit_rate;
    codec_context->rc_max_rate = codec_context->bit_rate;
    codec_context->rc_min_rate = codec_context->bit_rate;
    codec_context->rc_buffer_size = (int)codec_context_->bit_rate;
    codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (codec_context->codec_id == AV_CODEC_ID_H264) {
      av_opt_set(
        codec_context->priv_data, "preset", "ultrafast", 0);  // ultrafast
      av_opt_set(codec_context->priv_data, "tune", "zerolatency", 0);
      av_opt_set_int(codec_context->priv_data, "avcintra-class", -1, 0);
    }
    if (avcodec_open2(codec_context, codec_context->codec, nullptr) != 0) {
      avcodec_free_context(&codec_context);
      return false;
    }

    codec_context_ = codec_context;
    return true;
  }
  virtual bool prepare(AVConfig &config) = 0;
  virtual void close() = 0;
  virtual AVFramePtr decode(AVPacketPtr pkt) = 0;

  virtual uint32_t getFrameNum() const { return 0; }
  virtual void setBitRate(uint32_t bit_rate_bps) { }

  LY_NONCOPYABLE(Decoder);
protected:
  AVConfig config_;
  AVCodecContext *codec_context_{nullptr};
};

NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END
