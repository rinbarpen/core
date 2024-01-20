#pragma once
#include <cstdio>
#include <memory>
#include <core/util/marcos.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(ffmpeg)

enum RegFlag : int
{
  NETWORK = 1,
  DEVICE = 2,
  ALL = ~0,
};

static void version() {
  printf("ffmpeg version:\n");
  printf("\tlibavcodec   : %u\n", avcodec_version());
  printf("\tlibavdevice  : %u\n", avdevice_version());
  printf("\tlibavfilter  : %u\n", avfilter_version());
  printf("\tlibavformat  : %u\n", avformat_version());
  printf("\tlibavutil    : %u\n", avutil_version());
  printf("\tlibswresample: %u\n", swresample_version());
  printf("\tlibswscale   : %u\n", swscale_version());
}
static void reg_ffmpeg(int flags) {
  if ((flags & RegFlag::NETWORK) == RegFlag::NETWORK)
  {
    printf("Initialize network...\n");
    avformat_network_init();
    printf("Finish to initialize network\n");
  }
  if ((flags & RegFlag::DEVICE) == RegFlag::DEVICE)
  {
    printf("Initialize device...\n");
    avdevice_register_all();
    printf("Finish to initialize device\n");
  }
}

using AVPacketPtr = std::shared_ptr<AVPacket>;
using AVFramePtr = std::shared_ptr<AVFrame>;

inline AVPacketPtr makePacketPtr() {
  return AVPacketPtr(
    av_packet_alloc(), [](AVPacket *pkt) { av_packet_free(&pkt); });
}
inline AVFramePtr makeFramePtr() {
  return AVFramePtr(
    av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); });
}

struct AudioConfig
{
  uint32_t sample_rate;
  uint32_t bit_rate;
  uint32_t channels;

  AVSampleFormat format;
};

struct VideoConfig
{
  uint32_t width;
  uint32_t height;
  uint32_t frame_rate;
  uint32_t bit_rate;
  uint32_t gop;

  AVPixelFormat format;
};

struct CommonConfig
{
  // TODO: Add speed support
  float speed{ 1.0f };
};

struct AVConfig
{
  AudioConfig audio;
  VideoConfig video;
  CommonConfig common;
};

struct AVEncodeContext
{
  /* Video */
  const uint8_t *yuv;
  uint32_t width;
  uint32_t height;
  uint32_t image_size;

  /* Audio */
  const uint8_t *pcm;
  uint32_t samples;

  /* Common */
  uint32_t pts{};
};

struct AVDecodeContext
{
  /* Video */
  const uint8_t *yuv;
  uint32_t width;
  uint32_t height;
  uint32_t image_size;

  /* Audio */
  const uint8_t *pcm;
  uint32_t samples;

  /* Common */
  uint32_t pts{};
};

NAMESPACE_END(ffmpeg)
LY_NAMESPACE_END
namespace lyffmpeg = ly::ffmpeg;
