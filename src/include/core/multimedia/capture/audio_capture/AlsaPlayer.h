#pragma once
#include <functional>
#include <core/util/Mutex.h>
#include <core/util/thread/Thread.h>
#include <core/multimedia/capture/audio_capture/AVPlayer.h>
#ifdef __LINUX__
# include <alsa/asoundlib.h>
#endif

LY_NAMESPACE_BEGIN
class AlsaPlayer : public AVPlayer
{
public:
	using AudioDataCallback =
#ifdef __LINUX__
  std::function<void(const snd_pcm_t *mixFormat, uint8_t *data, uint32_t samples)>;
#else
  std::function<void(const void *mixFormat, uint8_t *data, uint32_t samples)>;
#endif

  AlsaPlayer();
  ~AlsaPlayer();

  bool init() override;
  bool destroy() override;

  bool start() override;
  bool stop() override;

  void setCallback(AudioDataCallback callback) { callback_ = callback; }

private:
  bool play() override;

private:
  bool initialized_{false};
  bool enabled_{false};
  Mutex::type mutex_;
  Thread thread_{"AlsaPlayer"};
  AudioDataCallback callback_;

#ifdef __LINUX__
  snd_pcm_hw_params_t *mix_format_;
  uint32_t buffer_frame_count_;
#endif
};
LY_NAMESPACE_END
