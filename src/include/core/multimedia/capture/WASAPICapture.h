#pragma once

#include <cstdio>
#include <cstdint>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include "core/util/marcos.h"

#ifdef __WIN__
LY_NAMESPACE_BEGIN
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include "core/multimedia/capture/AudioCapture.h"

class WASAPICapture : public AVCapture
{
public:
	using PacketCallback = std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)>;

	WASAPICapture();
	~WASAPICapture();

	bool init() override;
	void destroy() override;
	bool start() override;
	bool stop() override;

//	void setCallback(PacketCallback callback);

	// WAVEFORMATEX *getAudioFormat() const { return mix_format_; }


private:
	bool adjustFormatTo16Bits(WAVEFORMATEX *pwfx);
	bool capture() override;

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
	const int REFTIMES_PER_SEC = 10000000;
	const int REFTIMES_PER_MILLISEC = 10000;

	bool initialized_{false};
	bool enabled_{false};
	Mutex::type mutex_, mutex2_;
	std::thread worker_;
	WAVEFORMATEX *mix_format_;
	REFERENCE_TIME hnsActualDuration_;
	uint32_t buffer_frame_count_;
	std::shared_ptr<uint8_t> pcm_buf_;
	uint32_t pcm_buf_size;
  // SharedString pcm_buf_;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
	Microsoft::WRL::ComPtr<IMMDevice> device_;
	Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> audio_capture_client_;
};
LY_NAMESPACE_END
#endif
