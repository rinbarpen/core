#pragma once

#include <cstdint>
#include <functional>
#include <thread>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/multimedia/capture/AVPlayer.h>

#ifdef __WIN__
# include <wrl.h>
# include <Audioclient.h>
# include <mmdeviceapi.h>
#endif

LY_NAMESPACE_BEGIN
class WASAPIPlayer : public AVPlayer
{
public:
	using AudioDataCallback =
#ifdef __WIN__
  std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)>;
#else
  std::function<void(const void *mixFormat, uint8_t *data, uint32_t samples)>;
#endif

  WASAPIPlayer();
	~WASAPIPlayer();

	bool init() override;
  bool destroy() override;
  bool start() override;
	bool stop() override;

	void setCallback(AudioDataCallback callback);

  LY_NONCOPYABLE(WASAPIPlayer);
private:
	bool adjustFormatTo16Bits(
#ifdef __WIN__
		WAVEFORMATEX *
#else
		void *
#endif
		pwfx
  );
	bool play() override;

private:
#ifdef __WIN__
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	const int REFTIMES_PER_SEC = 10000000;
	const int REFTIMES_PER_MILLISEC = 10000;
#endif

	bool initialized_{false};
	bool enabled_{false};
	Mutex::type mutex_;
	std::thread worker_;
	AudioDataCallback callback_;

#ifdef __WIN__
  WAVEFORMATEX *mix_format_;
	REFERENCE_TIME hns_actual_duration_;
	uint32_t buffer_frame_count_;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
	Microsoft::WRL::ComPtr<IMMDevice> device_;
	Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
	Microsoft::WRL::ComPtr<IAudioRenderClient> audio_render_client_;
#endif
};
LY_NAMESPACE_END
