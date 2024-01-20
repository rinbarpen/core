#pragma once
#include <memory>

#include "core/multimedia/capture/AVCapture.h"
#include "core/multimedia/capture/AVPlayer.h"
#include "core/multimedia/capture/AudioBuffer.h"
#include "core/util/marcos.h"


LY_NAMESPACE_BEGIN
class AudioCapture
{
public:
  AudioCapture(AVPlayer *player, AVCapture *capture, uint32_t buffer_size = 20480);
  virtual ~AudioCapture();

  int read(uint8_t *data, uint32_t samples);

  int getSamples() const {
    return audio_buffer_->size() * 8 / bits_per_sample_ / channels_;
  }
  uint32_t getSamplerate() const { return sample_rate_; }
  uint32_t getChannels() const { return channels_; }
  uint32_t getBitsPerSample() const { return bits_per_sample_; }
  bool isCaptureStarted() const { return started_; }

protected:
  virtual void startCapture() = 0;
  virtual void stopCapture() = 0;

  bool started_{false};

  uint32_t channels_{2};
  uint32_t sample_rate_{48000};   // CD
  uint32_t bits_per_sample_{16};  // lr_pcm8

  AVPlayer *player_;
  AVCapture *capture_;
  std::unique_ptr<AudioBuffer> audio_buffer_;
};
LY_NAMESPACE_END
