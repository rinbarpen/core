#include <core/multimedia/capture/FFmpegAudioCapture.h>
#include <SDL2/SDL.h>

LY_NAMESPACE_BEGIN
FFmpegAudioCapture(std::unique_ptr<AVPlayer> player, std::unique_ptr<AVCapture> capture)
	: AudioCapture(std::move(player), std::move(capture))
{
	SDL_Init(SDL_INIT_AUDIO);
	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 44100; // 采样率
	want.format = AUDIO_S16SYS; // 采样格式
	want.channels = 2; // 通道数
	want.samples = 1024; // 每次回调函数调用的样本数
	want.callback = audioCallback;
}
bool FFmpegAudioCapture::startCapture()
{
	capture_->setCallback([this](const , uint8_t *data, uint32_t samples) {
#if 0
		static AVClock clock;
		static int samples_per_sec = 0;

		samples_per_sec += samples;
		if (clock.elapsed() >= 990) {
			ILOG_DEBUG(g_multimedia_logger) << "samples per second: " << samples_per_sec;
      samples_per_sec = 0;
			clock.reset();
		}
#endif
		channels_ = mixFormat->nChannels;
		sample_rate_ = mixFormat->nSamplesPerSec;
		bits_per_sample_ = mixFormat->wBitsPerSample;
		audio_buffer_->write(data, mixFormat->nBlockAlign * samples);
	});

	audio_buffer_->clear();
	if (!capture_->start()) {
		return false;
	}

	player_->start([this](const mixFormat, uint8_t *data, uint32_t samples) {
		memset(data, 0, mixFormat->nBlockAlign*samples);
	});


	started_ = true;
	return true;
}
bool FFmpegAudioCapture::stopCapture()
{

	return true;
}

LY_NAMESPACE_END
