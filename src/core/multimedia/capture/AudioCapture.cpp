#include "core/multimedia/capture/AudioCapture.h"

LY_NAMESPACE_BEGIN
AudioCapture::AudioCapture(std::unique_ptr<AVPlayer> player, std::unique_ptr<AVCapture> capture) :
  player_(std::move(player)),
	capture_(std::move(capture))
{}
AudioCapture::~AudioCapture()
{
}

bool AudioCapture::init(uint32_t buffer_size)
{
	if (initialized_) {
		return true;
	}

	if (!capture_->init()) {
		// throw ErrorCaptureInitialize(capture_->type());
		return false;
	}

	auto audio_format = capture_->getAudioFormat();
	channels_ = audio_format.channels;
	sample_rate_ = audio_format.samples_per_sec;
	bits_per_sample_ = audio_format.bits_per_sample;

	if(!player_->init()) {
		// throw ErrorPlayerInitialize(player_->type());
		return false;
	}
	audio_buffer_.reset(new AudioBuffer(buffer_size));
	if (!this->startCapture()) {
		// throw ErrorCaptureStart(capture_->type());
		return false;
	}

	initialized_ = true;
	return true;
}
void AudioCapture::destroy()
{
	if (initialized_) {
		this->stopCapture();
		initialized_ = false;
	}
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
