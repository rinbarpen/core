#pragma once

#include <cassert>
#include <stdexcept>

#include "ffmpeg_util.h"

namespace ffmpeg
{
struct AudioInfo
{
  int sample_rate;       /* 采样率 */
  int channels;          /* 声道数 */
  int bits_per_sample;   /* 每次采样的比特数 */
  AVSampleFormat format; /* 音频样式 */
};
class Resampler
{
public:
  Resampler(AudioInfo in, AudioInfo out)
  {
    swr_context_ = swr_alloc();
    if (nullptr == swr_context_) {
      throw std::runtime_error("swr_alloc");
    }
    int64_t in_ch_layout = av_get_default_channel_layout(in.channels);
    int64_t out_ch_layout = av_get_default_channel_layout(out.channels);

    av_opt_set_int(swr_context_, "in_sample_rate", in.sample_rate, 0);
    av_opt_set_channel_layout(swr_context_, "in_channel_layout", in_ch_layout, 0);
    av_opt_set_sample_fmt(swr_context_, "in_sample_fmt", in.format, 0);

    av_opt_set_int(swr_context_, "out_sample_rate", out.sample_rate, 0);
    av_opt_set_channel_layout(swr_context_, "out_channel_layout", out_ch_layout, 0);
    av_opt_set_sample_fmt(swr_context_, "out_sample_fmt", out.format, 0);

    if (swr_init(swr_context_) < 0) {
      throw std::runtime_error("swr_init");
    }
    in_audio_info_ = in;
    in_audio_info_.bits_per_sample = av_get_bytes_per_sample(in.format);
    out_audio_info_ = out;
    out_audio_info_.bits_per_sample = av_get_bytes_per_sample(out.format);
  }
  ~Resampler()
  {
    if (swr_context_) {
      if (swr_is_initialized(swr_context_))
        swr_close(swr_context_);
      swr_free(&swr_context_);
      swr_context_ = nullptr;
    }
  }

  int convert(AVFramePtr in_frame, AVFramePtr &out_frame)
  {
    assert(swr_context_ != nullptr);

    out_frame = makeFramePtr();
    out_frame->sample_rate = out_audio_info_.sample_rate;
    out_frame->channels = out_audio_info_.channels;
    out_frame->format = out_audio_info_.format;
    out_frame->nb_samples = av_rescale_rnd(in_frame->nb_samples, out_frame->sample_rate, in_frame->sample_rate, AV_ROUND_UP);
    out_frame->pts = out_frame->pkt_dts = in_frame->pts;

    if (av_frame_get_buffer(out_frame.get(), 0) != 0) {
      return -1;
    }

    int len = swr_convert(swr_context_, 
        (uint8_t**)&out_frame->data, out_frame->nb_samples,
        (const uint8_t**)&in_frame->data, in_frame->nb_samples);
    if (len < 0) {
      out_frame = nullptr;
      return -1;
    }

    return len;
  }

  Resampler(const Resampler &) = delete;
  Resampler& operator=(const Resampler &) = delete;
protected:
  SwrContext *swr_context_;

  /* in */
  AudioInfo in_audio_info_;
  /* out */
  AudioInfo out_audio_info_;
};

}
