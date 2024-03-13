#pragma once

#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/capture/screen_capture/ScreenCapture.h>
#include <core/util/Mutex.h>
#include <core/util/ds/SharedString.h>
#include <core/util/thread/Thread.h>

#ifdef __LINUX__
# include <X11/X.h>
#endif

LY_NAMESPACE_BEGIN
class X11ScreenCapture : public ScreenCapture
{
public:
	X11ScreenCapture(uint32_t width, uint32_t height, uint32_t offset_x = 0, uint32_t offset_y = 0);
	virtual ~X11ScreenCapture();

	bool init(int display_index = 0, bool auto_run = true) override;
	bool destroy() override;

	bool captureFrame(std::vector<uint8_t>& image, uint32_t& width, uint32_t& height) override;

	uint32_t getWidth() const override;
	uint32_t getHeight() const override;
	bool isCapturing() const override;

private:
	bool startCapture();
	void stopCapture();
	bool acquireFrame();
	bool decode(ffmpeg::AVFramePtr frame, ffmpeg::AVPacketPtr packet);

private:
	uint32_t offset_x_;
	uint32_t offset_y_;
	int display_index_;

	bool initialized_{false};
	bool started_{false};
	Thread worker_{"X11ScreenCapture"};

	AVFormatContext* format_context_{nullptr};
	AVInputFormat* input_format_{nullptr};
	AVCodecContext* codec_context_{nullptr};
	int video_index_ = -1;
	int frame_rate_ = 25;

	Mutex::type mutex_;
  SharedString<uint8_t> image_;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
};
LY_NAMESPACE_END
