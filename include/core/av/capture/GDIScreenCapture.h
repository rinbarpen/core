#pragma once

#include <thread>

#include "Mutex.h"
#include "ScreenCapture.h"
#include "WindowsHelper.h"
#include "ffmpeg/ffmpeg_util.h"

LY_NAMESPACE_BEGIN
class GDIScreenCapture : public ScreenCapture
{
public:
  GDIScreenCapture(int display_index = 0);
  virtual ~GDIScreenCapture();

  // SharedString
  virtual bool captureFrame(std::vector<uint8_t>& image, uint32_t& width, uint32_t& height) override;

  virtual bool isCapturing() const override { return capturing_; }
  virtual uint32_t getWidth() const override { return width_; }
  virtual uint32_t getHeight() const override { return height_; }

private:
  bool startCapture();
  void stopCapture();
  bool acquireFrame();
  bool decode(ffmpeg::AVFramePtr pFrame, ffmpeg::AVPacketPtr pPacket);

private:
  bool capturing_{false};

  DX::Monitor monitor_;

  // use XThread instead
  std::thread thread_;
  AVFormatContext *format_context_ = nullptr;
  const AVInputFormat *input_format_ = nullptr;
  AVCodecContext *codec_context_ = nullptr;
  int video_index_{-1};
  int frame_rate_{25};

  // use SharedString instead
  std::shared_ptr<uint8_t> image_;
  uint32_t image_size_;

  uint32_t width_;
  uint32_t height_;

  Mutex::type mutex_;
};
LY_NAMESPACE_END
