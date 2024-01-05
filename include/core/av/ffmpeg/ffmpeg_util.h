#pragma once
#include <cstdio>
#include <memory>

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

namespace ly
{
namespace ffmpeg
{
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

static inline AVPacketPtr makePacketPtr() {
  return AVPacketPtr(
    av_packet_alloc(), [](AVPacket *pkt) { av_packet_free(&pkt); });
}
static inline AVFramePtr makeFramePtr() {
  return AVFramePtr(
    av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); });
}

}  // namespace ffmpeg
}  // namespace ly
namespace lyffmpeg = ly::ffmpeg;
