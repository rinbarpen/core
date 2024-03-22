#pragma once
#include <functional>
#include <core/util/Mutex.h>
#include <core/util/ds/SharedString.h>
#include <core/multimedia/capture/audio_capture/AVCapture.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/util/thread/Thread.h>

#ifdef __LINUX__
# include <alsa/asoundlib.h>
# include <alsa/pcm.h>
#endif

LY_NAMESPACE_BEGIN
class AlsaCapture : public AVCapture
{
public:
#ifdef __LINUX__
  using PacketCallback = std::function<void(const snd_pcm_t *mixFormat, uint8_t *data, uint32_t samples)>;
#else
  using PacketCallback = std::function<void(const void *mixFormat, uint8_t *data, uint32_t samples)>;
#endif

  AlsaCapture();
  ~AlsaCapture();

  bool init() override;
  bool destroy() override;

  bool start() override;
  bool stop() override;

  void setCallback(PacketCallback callback);
  AudioFormat getAudioFormat() const override;

private:
	bool capture() override;

private:
  Mutex::type mutex_;
  bool initialized_{false};
  bool enabled_{false};
  Mutex::type mutex2_;
	PacketCallback callback_;

  Thread thread_{"AlsaCapture"};

  snd_pcm_t *handle_{nullptr};

	AVInputFormat* input_format_{nullptr};
	AVFormatContext* format_context_{nullptr};
	AVCodecContext* codec_context_{nullptr};
  int stream_index_{-1};

  SharedString<uint8_t> pcm_buffer_;

  static constexpr int kChannels = 2;
  static constexpr int kSampleRate = 48000;
  // static constexpr int kFSize = 2 * kChannels;
};
LY_NAMESPACE_END
