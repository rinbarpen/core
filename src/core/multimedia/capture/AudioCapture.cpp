#include "core/multimedia/capture/AudioCapture.h"

LY_NAMESPACE_BEGIN
AudioCapture::AudioCapture(AVPlayer *player, AVCapture *capture, uint32_t buffer_size) :
  player_(player), capture_(capture),
  audio_buffer_(new AudioBuffer(buffer_size))
{}
AudioCapture::~AudioCapture()
{
  delete player_;
	delete capture_;
}

int AudioCapture::read(uint8_t* data, uint32_t samples)
{
	if (samples > this->getSamples()) {
		return 0;
	}

	audio_buffer_->read(data, samples * bits_per_sample_ / 8 * channels_);
	return samples;
}
LY_NAMESPACE_END
