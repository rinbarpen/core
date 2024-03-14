#pragma once

#include <functional>
#include <core/util/Mutex.h>
#include <core/multimedia/capture/audio_capture/AVCapture.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#ifdef __LINUX__
# include <pulse/pulseaudio.h>
#endif

LY_NAMESPACE_BEGIN
// TODO: Complete me!
class PulseCapture : public AVCapture
{
public:
#ifdef __LINUX__
  using PacketCallback = std::function<void(pa_sample_spec *handle, int buffer_frames)>;
#else
  using PacketCallback = std::function<void(void *handle, int buffer_frames)>;
#endif

  PulseCapture();
  ~PulseCapture();

  bool init() override;
  bool destroy() override;

  bool start() override;
  bool stop() override;

  AudioFormat getAudioFormat() const override;

  void setCallback(PacketCallback callback) { callback_ = callback; }

private:
  bool capture() override;

private:
  Mutex::type mutex_;
  bool initialized_{false};
  bool enabled_{false};
	PacketCallback callback_;

};
LY_NAMESPACE_END
