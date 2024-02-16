#include <thread>

#include <core/util/Mutex.h>
#include <core/util/ds/SharedString.h>
#include <core/multimedia/capture/ScreenCapture.h>
#include <core/multimedia/capture/WASAPIHelper.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
class GDIScreenCapture : public ScreenCapture
{
public:
	GDIScreenCapture();
	virtual ~GDIScreenCapture();

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

	DX::Monitor monitor_;

	bool initialized_{false};
	bool started_{false};
	std::thread worker_;

	AVFormatContext* format_context_{nullptr};
	AVInputFormat* input_format_{nullptr};
	AVCodecContext* codec_context_{nullptr};
	int video_index_ = -1;
	int frame_rate_ = 25;

	Mutex::type mutex_;
  SharedString image_;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
};
LY_NAMESPACE_END
