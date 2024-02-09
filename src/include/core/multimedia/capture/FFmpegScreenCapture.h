#pragma once
#include "core/multimedia/util/AVQueue.h"
#include "core/util/marcos.h"
#include <core/multimedia/ffmpeg/ffmpeg_util.h>
#include <core/multimedia/capture/ScreenCapture.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN

class FFmpegScreenCapture final : public ScreenCapture
{
public:
  // FFmpegScreenCapture(const char *video_device)
  //   : pkt_queue_(new AVPacketQueue())
  // {
  //   pkt_queue_->open();

  //   auto input_fmt = av_find_input_format("v4l2");
  //   if (0 != ::avformat_open_input(&fmt_context_, video_device, input_fmt, nullptr)) {
  //     ILOG_ERROR_FMT(g_multimedia_logger, "Error opening video device: {}", video_device);
  //     ILOG_ERROR_FMT(g_multimedia_logger, "Are you sure that it is successful to initialize avdevice first?");
  //     throw 0;  // TODO: Use exception instead
  //   }

  //   ::av_dump_format(fmt_context_, 0, video_device, 0);
  // }
  explicit FFmpegScreenCapture(const char *vcodec);
  ~FFmpegScreenCapture();
  // ~FFmpegScreenCapture() {
  //   if (fmt_context_)
  //     ::avformat_close_input(&fmt_context_);
  // }

  bool init(int display_index = 0) override;
  bool destroy() override;
  bool captureFrame(std::vector<uint8_t> &image, uint32_t &width, uint32_t &height) override;

  bool isCapturing() const override;
  uint32_t getWidth() const override;
  uint32_t getHeight() const override;

  // void captureFrame() {
  //   ffmpeg::AVPacketPtr pkt = ffmpeg::makePacketPtr();

  //   while (true) {
  //     if (0 != av_read_frame(fmt_context_, pkt.get())) {
  //       // error on reading frame
  //       break;
  //     }
  //     bool r = pkt_queue_->push(pkt);
  //     if (!r) {
  //       break;
  //     }
  //   }
  // }

private:
  bool initialized_{false};
  bool capturing_{false};
  AVFormatContext *fmt_context_{nullptr};
  std::shared_ptr<AVPacketQueue> pkt_queue_;
};
LY_NAMESPACE_END
