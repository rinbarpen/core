#pragma once
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

namespace ffmpeg
{
extern "C"
{
static void version()
{
  printf("version:\n");
  printf("\tlibavcodec   : %s\n", avcodec_version());
  printf("\tlibavdevice  : %s\n", avdevice_version());
  printf("\tlibavfilter  : %s\n", avfilter_version());
  printf("\tlibavformat  : %s\n", avformat_version());
  printf("\tlibavutil    : %s\n", avutil_version());
  printf("\tlibswresample: %s\n", swresample_version());
  printf("\tlibswscale   : %s\n", swscale_version());
}
static void reg_ffmpeg() 
{
  // avformat_network_init();
  avdevice_register_all();
}
}

using AVPacketPtr = std::shared_ptr<AVPacket>;
using AVFramePtr  = std::shared_ptr<AVFrame>;

static inline AVPacketPtr makePacketPtr()
{
  return AVPacketPtr(av_packet_alloc(), [](AVPacket *pkt) {
    av_packet_free(&pkt);
  });
}
static inline AVFramePtr makeFramePtr() {
  return AVFramePtr(av_frame_alloc(), [](AVFrame *frame) {
    av_frame_free(&frame);
  });
}

}  // namespace ffmpeg
