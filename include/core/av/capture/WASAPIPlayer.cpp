#include "WASAPIPlayer.h"
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
static auto g_av_logger = GET_LOGGER("av");

void WASAPIPlayer::WASAPIPlayer() 
{
	CoInitialize(NULL);

	HRESULT hr = S_OK;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)enumerator_.GetAddressOf());
	if (FAILED(hr)) {
	  throw std::runtime_error("[WASAPIPlayer] Failed to create instance.");
	}

	hr = enumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, device_.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to create device.");
	}

	hr = device_->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void **)audio_client_.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to activate device.");
	}

	hr = audio_client_->GetMixFormat(&mix_format_);
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to get mix format.");
	}

	adjustFormatTo16Bits(mix_format_);
	hns_actual_duration_ = REFTIMES_PER_SEC;
	hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hns_actual_duration_, 0, mix_format_, NULL);
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to initialize audio client.");
	}

	hr = audio_client_->GetBufferSize(&buffer_frame_count_);
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to get buffer size.");
	}

	hr = audio_client_->GetService(IID_IAudioRenderClient, (void **)audio_render_client_.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to get service.");
	}

	// Calculate the actual duration of the allocated buffer.
	hns_actual_duration_ = REFTIMES_PER_SEC * buffer_frame_count_ / mix_format_->nSamplesPerSec;
}

void WASAPIPlayer::~WASAPIPlayer() 
{
	CoUninitialize();
}

void WASAPIPlayer::start()
{
	Mutex::lock locker(mutex_);
	if (enabled_) return ;

	HRESULT hr = audio_client_->Start();
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to start audio client.");
		return ;
	}
	
	thread_ = std::thread([this] {
		while (enabled_) {
			try {
				this->play();
			} catch(std::runtime_error &e) {
			  ILOG_ERROR(g_av_logger) << e.what();
				break;
			}
		}
	});
	enabled_ = true;
}

void WASAPIPlayer::stop()
{
	Mutex::lock locker(mutex_);
  if (!enabled_) { return ; } 

  LY_ASSERT(thread_.joinable());
	thread_.join();

	HRESULT hr = audio_client_->Stop();
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to stop audio client.");
		return ;
	}

	enabled_ = false;
}

bool WASAPIPlayer::adjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
		if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
			pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			pEx->Samples.wValidBitsPerSample = 16;
		}
	}
	else {
		return false;
	}

	pwfx->wBitsPerSample = 16;
	pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
	pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
	return true;
}

void WASAPIPlayer::play()
{
	uint32_t numFramesPadding = 0;
	uint32_t numFramesAvailable = 0;

	HRESULT hr = audio_client_->GetCurrentPadding(&numFramesPadding);
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPIPlayer] Failed to get current padding.");
		return ;
	}

	numFramesAvailable = buffer_frame_count_ - numFramesPadding;

	if (numFramesAvailable > 0) {
		BYTE *data;
		hr = audio_render_client_->GetBuffer(numFramesAvailable, &data);
		if (FAILED(hr)) {
			throw std::runtime_error("[WASAPIPlayer] Audio render client failed to get buffer.");
			return ;
		}

		if (callback_) {
			memset(data, 0, mix_format_->nBlockAlign * numFramesAvailable);
			callback_(mix_format_, data, numFramesAvailable);
		}

		hr = audio_render_client_->ReleaseBuffer(numFramesAvailable, 0);
		if (FAILED(hr)) {
			throw std::runtime_error("[WASAPIPlayer] Audio render client failed to release buffer.");
			return;
		}
	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
LY_NAMESPACE_END
