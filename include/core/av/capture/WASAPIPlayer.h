#pragma once

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>

#include "AVPlayer.h"
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
class WASAPIPlayer : public AVPlayer
{
public:
	using AudioDataCallback = std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)>;

	WASAPIPlayer();
	~WASAPIPlayer();

	void start() override;
  void stop() override;

  void setCallback(AudioDataCallback callback) { callback_ = callback; }

private:
	bool adjustFormatTo16Bits(WAVEFORMATEX *pwfx);
	void play() override;

private:
	static constexpr CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	static constexpr IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	static constexpr IID IID_IAudioClient = __uuidof(IAudioClient);
	static constexpr IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	static constexpr int REFTIMES_PER_SEC = 10000000;
	static constexpr int REFTIMES_PER_MILLISEC = 10000;

	WAVEFORMATEX *mix_format_;
	REFERENCE_TIME hns_actual_duration_;
	AudioDataCallback callback_;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
	Microsoft::WRL::ComPtr<IMMDevice> device_;
	Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
	Microsoft::WRL::ComPtr<IAudioRenderClient> audio_render_client_;

	Mutex::type mutex_;
  std::thread thread_;
};
LY_NAMESPACE_END
