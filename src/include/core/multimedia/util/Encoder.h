#pragma once

#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/Resampler.h>


LY_NAMESPACE_BEGIN
#if 0
struct EncoderConfig
{
  struct video {
    const uint8_t *yuv;
    uint32_t width;
    uint32_t height;
    AVPixelFormat format;
    // uint32_t image_size;  // size(format) * width * height

    // bits
    uint32_t imageSize() const { return width * height * av_get_bits_per_pixel(av_pix_fmt_desc_get(format)); }
  } video;

  struct audio {
    const uint8_t *pcm;
    uint32_t samples_per_channel;
    uint32_t nchannels;
    AVSampleFormat format;

    uint32_t nsamples() const { return samples_per_channel * nchannels; }
    uint32_t audioSize() const { return samples_per_channel * nchannels * av_get_bytes_per_sample(format); }
  } audio;

  struct common {
    int pts;
  } common;
};

class Encoder {
public:
  Encoder() = default;
  virtual ~Encoder() = default;

  virtual bool open(ffmpeg::AVConfig &config) = 0;
  virtual bool close() = 0;

  virtual ffmpeg::AVPacketPtr encode(EncoderConfig config) { return nullptr; }
  virtual ffmpeg::AVPacketPtr encode(ffmpeg::AVFramePtr pFrame, EncoderConfig config) { return nullptr; }

  LY_NONCOPYABLE(Encoder);
protected:
  AVCodecContext *codec_context_;
  ffmpeg::AVConfig config_;
  std::unique_ptr<ffmpeg::Resampler> resampler_;
  int64_t pts_;
};
#endif
LY_NAMESPACE_END
