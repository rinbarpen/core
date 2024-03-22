#include <memory>
#include <core/util/time/Timestamp.h>
#include <core/multimedia/capture/audio_capture/AlsaAudioCapture.h>

LY_NAMESPACE_BEGIN
AlsaAudioCapture::AlsaAudioCapture()
  : AudioCapture(
    std::make_unique<AlsaPlayer>(), std::make_unique<AlsaCapture>()) {}
AlsaAudioCapture::~AlsaAudioCapture() {
  destroy();
}

bool AlsaAudioCapture::startCapture() {
#ifdef __LINUX__
  auto pAlsaCapture = dynamic_cast<AlsaCapture *>(capture_.get());
  pAlsaCapture->setCallback(
    [this](const snd_pcm_t *mixFormat, uint8_t *data, uint32_t samples) {
#if 1
		static Timestamp timestamp;
		static int samplesPerSecond = 0;

		samplesPerSecond += samples;
		if (timestamp.elapsed().count() >= 990) {
			//printf("samples per second: %d\n", samplesPerSecond);
			samplesPerSecond = 0;
			timestamp.reset();
		}
#endif
      // channels_ = mixFormat->nChannels;
      // sample_rate_ = mixFormat->nSamplesPerSec;
      // bits_per_sample_ = mixFormat->wBitsPerSample;
      // audio_buffer_->write(data, mixFormat->nBlockAlign * samples);
    });

  audio_buffer_->clear();
  if (!capture_->start()) {
    return false;
  }

  auto pAlsaPlayer = dynamic_cast<AlsaPlayer *>(player_.get());
  pAlsaPlayer->setCallback(
    [this](const snd_pcm_t *mixFormat, uint8_t *data, uint32_t samples) {
      // memset(data, 0, mixFormat->nBlockAlign * samples);
    });
  pAlsaPlayer->start();

  started_ = true;
  return true;
#else
  return false;
#endif
}
bool AlsaAudioCapture::stopCapture() {
	player_->stop();
	capture_->stop();
	started_ = false;
	return true;
}

LY_NAMESPACE_END