#include <core/multimedia/capture/audio_capture/WASAPIAudioCapture.h>

LY_NAMESPACE_BEGIN
WASAPIAudioCapture::WASAPIAudioCapture()
  : AudioCapture(std::make_unique<WASAPIPlayer>(), std::make_unique<WASAPICapture>())
{}
WASAPIAudioCapture::~WASAPIAudioCapture()
{
  this->destroy();
}

bool WASAPIAudioCapture::startCapture() {
#ifdef __WIN__
	auto pWASCapture = dynamic_cast<WASAPICapture*>(capture_.get());
  pWASCapture->setCallback([this](const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples) {
# if 0
		static Timestamp timestamp;
		static int samplesPerSecond = 0;

		samplesPerSecond += samples;
		if (timestamp.elapsed().count() >= 990) {
			//printf("samples per second: %d\n", samplesPerSecond);
			samplesPerSecond = 0;
			timestamp.reset();
		}
# endif
		channels_ = mixFormat->nChannels;
		sample_rate_ = mixFormat->nSamplesPerSec;
		bits_per_sample_ = mixFormat->wBitsPerSample;
		audio_buffer_->write(data, mixFormat->nBlockAlign * samples);
	});

	audio_buffer_->clear();
	if (!capture_->start()) {
		return false;
	}

  auto pWASPlayer = dynamic_cast<WASAPIPlayer*>(player_.get());
  pWASPlayer->setCallback([this](const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples) {
    memset(data, 0, mixFormat->nBlockAlign * samples);
  });
  pWASPlayer->start();

	started_ = true;
	return true;
#else
  return false;
#endif
}
bool WASAPIAudioCapture::stopCapture() {
#ifdef __WIN__
	player_->stop();
	capture_->stop();
	started_ = false;
	return true;
#else
  return false;
#endif
}
LY_NAMESPACE_END
