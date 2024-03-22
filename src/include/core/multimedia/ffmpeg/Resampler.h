#pragma once

#include <stdexcept>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)
struct AudioInfo
{
  int sample_rate;
  int channels;
  int bits_per_sample;
  AVSampleFormat format;

  bool operator==(const AudioInfo &rhs) const {
    return sample_rate == rhs.sample_rate
        && channels == rhs.channels
        && bits_per_sample == rhs.bits_per_sample
        && format == rhs.format;
  }
  bool operator!=(const AudioInfo &rhs) const {
    return !(*this == rhs);
  }
};
// pcm lr 8 44100
/**
 * @brief Resample the audio(pcm) data
 *
 * @note Throw InitializationException when ffmpeg is loaded or memory is not enough
 *
 */
class Resampler
{
public:
  Resampler(AudioInfo in, AudioInfo out)
  {
    swr_context_ = swr_alloc();
    if (nullptr == swr_context_) {
      throw std::runtime_error("Out of memory in allocating swr_context");
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
      throw std::runtime_error("Fail to call swr_init");
    }
    in_audio_info_ = in;
    in_audio_info_.bits_per_sample = av_get_bytes_per_sample(in.format);
    out_audio_info_ = out;
    out_audio_info_.bits_per_sample = av_get_bytes_per_sample(out.format);
  }
  ~Resampler() noexcept
  {
    if (swr_context_) {
      if (swr_is_initialized(swr_context_))
        swr_close(swr_context_);
      swr_free(&swr_context_);
      // swr_context_ = nullptr;
    }
  }

  int convert(AVFramePtr in_frame, AVFramePtr &out_frame)
  {
    LY_ASSERT(swr_context_ != nullptr);

    if (nullptr == out_frame) {
      out_frame = makeFramePtr();
    }

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

  LY_NONCOPYABLE(Resampler);
protected:
  SwrContext *swr_context_;

  /* in */
  AudioInfo in_audio_info_;
  /* out */
  AudioInfo out_audio_info_;
};
NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END

#include <fmt/core.h>
#include <fmt/args.h>
#ifdef FMT_VERSION
template <>
struct fmt::formatter<::ly::ffmpeg::AudioInfo>
{
  constexpr auto parse(fmt::format_parse_context &ctx) {
   return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const ::ly::ffmpeg::AudioInfo &obj, FormatContext &ctx) {
    return fmt::format_to(ctx.out(),
      "sample rate = {sample rate}\n"
      "channels = {channels}\n"
      "bits per sample = {bits per sample}\n"
      "sample format = {format}\n" ,
      fmt::arg("sample rate", obj.sample_rate),
      fmt::arg("channels", obj.channels),
      fmt::arg("bits per sample", obj.bits_per_sample),
      fmt::arg("format", av_get_sample_fmt_name(obj.format)));
  }
};
#endif
