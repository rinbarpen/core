#pragma once
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <core/util/Mutex.h>
#include "AVCapture.h"

LY_NAMESPACE_BEGIN
class WASAPICapture : public AVCapture
{
public:
  using PacketCallback = std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)>;

  WASAPICapture();
	~WASAPICapture();

	void start() override;
	void stop() override;
	void setCallback(PacketCallback callback) { callback_ = callback; }

	WAVEFORMATEX *getAudioFormat() const { return mix_format_; }

private:
	bool adjustFormatTo16Bits(WAVEFORMATEX *pwfx);
	bool capture() override;

private:
	static constexpr CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	static constexpr IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	static constexpr IID IID_IAudioClient = __uuidof(IAudioClient);
	static constexpr IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
	static constexpr int REFTIMES_PER_SEC = 10000000;
	static constexpr int REFTIMES_PER_MILLISEC = 10000;

	WAVEFORMATEX *mix_format_{nullptr};
	REFERENCE_TIME hns_actual_duration_;
	PacketCallback callback_;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
	Microsoft::WRL::ComPtr<IMMDevice> device_;
	Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> audio_capture_client_;

	std::thread thread_;
  bool enabled_{false};
	std::shared_ptr<uint8_t> pcm_buf_;
  uint32_t pcm_buf_size_;
	Mutex::type mutex_;
};
LY_NAMESPACE_END
