#pragma once

#include <cstdint>
#include <thread>
#include <memory>
#include <functional>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/multimedia/capture/AudioCapture.h>

#ifdef __WIN__
# include <wrl.h>
# include <mmdeviceapi.h>
# include <Audioclient.h>
#endif

LY_NAMESPACE_BEGIN
class WASAPICapture : public AVCapture
{
public:
#ifdef __WIN__
	using PacketCallback = std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)>;
#else
	using PacketCallback = std::function<void(const void *mixFormat, uint8_t *data, uint32_t samples)>;
#endif

	WASAPICapture();
	~WASAPICapture();

	bool init() override;
	bool destroy() override;
	bool start() override;
	bool stop() override;

	void setCallback(PacketCallback callback);
	AudioFormat getAudioFormat() const override;

	LY_NONCOPYABLE(WASAPICapture);
private:
	bool adjustFormatTo16Bits(
#ifdef __WIN__
		WAVEFORMATEX *
#else
		void *
#endif
		pwfx
	);
	bool capture() override;

private:
#ifdef __WIN__
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
	const int REFTIMES_PER_SEC = 10000000;
	const int REFTIMES_PER_MILLISEC = 10000;
#endif

	bool initialized_{false};
	bool enabled_{false};
	Mutex::type mutex_, mutex2_;
	std::thread worker_;
	uint32_t buffer_frame_count_;
	std::shared_ptr<uint8_t> pcm_buf_;
	uint32_t pcm_buf_size_;
	PacketCallback callback_;
#ifdef __WIN__
	WAVEFORMATEX *mix_format_;
	REFERENCE_TIME hns_actual_duration_;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
	Microsoft::WRL::ComPtr<IMMDevice> device_;
	Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> audio_capture_client_;
#endif
};
LY_NAMESPACE_END
