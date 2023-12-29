#include "WASAPICapture.h"

#include "util/logger/Logger.h"

LY_NAMESPACE_BEGIN
WASAPICapture::WASAPICapture()
{
	pcm_buf_size_ = 48000 * 32 * 2 * 2;
	pcm_buf_.reset(new uint8_t[pcm_buf_size_], std::default_delete<uint8_t[]>());

	CoInitialize(NULL);

  HRESULT hr = S_OK;
  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
    IID_IMMDeviceEnumerator, (void **) enumerator_.GetAddressOf());
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to create instance.");
  }

  hr = enumerator_->GetDefaultAudioEndpoint(
    eRender, eMultimedia, device_.GetAddressOf());
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to create device.");
  }

  hr = device_->Activate(
    IID_IAudioClient, CLSCTX_ALL, NULL, (void **) audio_client_.GetAddressOf());
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to activate device.");
  }

  hr = audio_client_->GetMixFormat(&mix_format_);
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to get mix format.");
  }

  adjustFormatTo16Bits(mix_format_);
  hns_actual_duration_ = REFTIMES_PER_SEC;
  hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK, hns_actual_duration_, 0, mix_format_, NULL);
  if (FAILED(hr))
  {
    throw std::runtime_error(
      "[WASAPICapture] Failed to initialize audio client.");
  }

  hr = audio_client_->GetBufferSize(&buffer_frame_count_);
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to get buffer size.");
  }

  hr = audio_client_->GetService(
    IID_IAudioCaptureClient, (void **) audio_capture_client_.GetAddressOf());
  if (FAILED(hr))
  {
    throw std::runtime_error("[WASAPICapture] Failed to get service.");
  }

  // Calculate the actual duration of the allocated buffer.
  hns_actual_duration_ = REFERENCE_TIME(
    REFTIMES_PER_SEC * buffer_frame_count_ / mix_format_->nSamplesPerSec);
}
WASAPICapture::~WASAPICapture()
{
  CoUninitialize();
}

// TODO: Use CaptureLog instead
// TODO: Use CaptureException instead

void WASAPICapture::start()
{
	Mutex::lock locker(mutex_);
  if (enabled_) return;

	HRESULT hr = audio_client_->Start();
	if (FAILED(hr)) {
    throw std::runtime_error("[WASAPICapture] Failed to start audio client.");
	}

	thread_ = std::thread([this] {
		while (enabled_) {
			if (!this->capture()) break;
		}
	});
	enabled_ = true;
}
void WASAPICapture::stop()
{
	Mutex::lock locker(mutex_);
	if (!enabled_) { return ; }

	LY_ASSERT(thread_.joinable());
	thread_.join();

	HRESULT hr = audio_client_->Stop();
	if (FAILED(hr)) {
		throw std::runtime_error("[WASAPICapture] Failed to stop audio client.");
	}
	enabled_ = false;
}

bool WASAPICapture::adjustFormatTo16Bits(WAVEFORMATEX* pwfx)
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

bool WASAPICapture::capture()
{
	HRESULT hr = S_OK;
	uint32_t packetLength = 0;
	uint32_t numFramesAvailable = 0;
	BYTE *pData;
	DWORD flags;

	hr = audio_capture_client_->GetNextPacketSize(&packetLength);
	if (FAILED(hr))
	{
		throw std::runtime_error("[WASAPICapture] Faild to get next data packet size.");
		return false;
	}

	if (packetLength == 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return true;
	}

	while (packetLength > 0) {
		hr = audio_capture_client_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
		if (FAILED(hr)) {
			throw std::runtime_error("[WASAPICapture] Faild to get buffer.");
			return false;
		}

		if (pcm_buf_size_ < numFramesAvailable * mix_format_->nBlockAlign) {
			pcm_buf_size_ = numFramesAvailable * mix_format_->nBlockAlign;
			pcm_buf_.reset(new uint8_t[pcm_buf_size_], std::default_delete<uint8_t[]>());
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
			memset(pcm_buf_.get(), 0, pcm_buf_size_);
		}
		else {
			memcpy(pcm_buf_.get(), pData, numFramesAvailable * mix_format_->nBlockAlign);
		}

		if (callback_) {
			callback_(mix_format_, pData, numFramesAvailable);
		}

		hr = audio_capture_client_->ReleaseBuffer(numFramesAvailable);
		if (FAILED(hr)) {
			throw std::runtime_error("[WASAPICapture] Faild to release buffer.");
			return false;
		}

		hr = audio_capture_client_->GetNextPacketSize(&packetLength);
		if (FAILED(hr)) {
			throw std::runtime_error("[WASAPICapture] Faild to get next data packet size.");
			return false;
		}
	}

	return true;
}
LY_NAMESPACE_END
