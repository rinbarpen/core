#pragma once

#include <cstdint>
#include <memory>
#include <core/multimedia/capture/AVCapture.h>
#include <core/multimedia/capture/AVPlayer.h>
#include <core/multimedia/capture/AudioBuffer.h>

LY_NAMESPACE_BEGIN
class AudioCapture
{
public:
  AudioCapture(std::unique_ptr<AVPlayer> player, std::unique_ptr<AVCapture> capture);
  virtual ~AudioCapture();

  bool init(uint32_t buffer_size = 20480);
  bool destroy();

  int read(uint8_t *data, uint32_t samples);

  int getSamples() const {
    return audio_buffer_->size() * 8 / bits_per_sample_ / channels_;
  }
  uint32_t getSampleRate() const { return sample_rate_; }
  uint32_t getChannels() const { return channels_; }
  uint32_t getBitsPerSample() const { return bits_per_sample_; }
  bool isCaptureStarted() const { return started_; }

protected:
  virtual bool startCapture() = 0;
  virtual bool stopCapture() = 0;

  bool initialized_{false};
  bool started_{false};

  uint32_t channels_{2};
  uint32_t sample_rate_{48000};   // CD
  uint32_t bits_per_sample_{16};  // lr_pcm8

  std::unique_ptr<AVPlayer> player_;
  std::unique_ptr<AVCapture> capture_;
  std::unique_ptr<AudioBuffer> audio_buffer_;
};
LY_NAMESPACE_END
