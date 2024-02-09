#include <core/multimedia/capture/WASAPICapture.h>

#ifdef __WIN__
LY_NAMESPACE_BEGIN
static auto g_multimedia_logger = GET_LOGGER("multimedia");

WASAPICapture::WASAPICapture()
  : pcm_buf_size(48000 * 32 * 2 * 2),
    pcm_buf_(new uint8_t[48000 * 32 * 2 * 2], std::default_delete<uint8_t[]>())
{
}

WASAPICapture::~WASAPICapture()
{

}

bool WASAPICapture::init()
{
	Mutex::lock locker(m_mutex);
	if (initialized_) return true;

	CoInitialize(NULL);

	HRESULT hr = S_OK;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)enumerator_.GetAddressOf());
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to create instance.";
		return false;
	}

	hr = enumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, device_.GetAddressOf());
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to create device.";
		return false;
	}

	hr = device_->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)audio_client_.GetAddressOf());
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to activate device.";
		return false;
	}

	hr = audio_client_->GetMixFormat(&mix_format_);
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to get mix format.";
		return false;
	}

	adjustFormatTo16Bits(mix_format_);
	m_hnsActualDuration = REFTIMES_PER_SEC;
	hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, m_hnsActualDuration, 0, mix_format_, NULL);
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to initialize audio client.";
		return false;
	}

	hr = audio_client_->GetBufferSize(&m_bufferFrameCount);
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to get buffer size.";
    return false;
	}

	hr = audio_client_->GetService(IID_IAudioCaptureClient, (void**)audio_capture_client_.GetAddressOf());
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to get service.";
		return false;
	}

	// Calculate the actual duration of the allocated buffer.
	hnsActualDuration = REFERENCE_TIME(REFTIMES_PER_SEC * m_bufferFrameCount / mix_format_->nSamplesPerSec);

	initialized_ = true;
	return true;
}

void WASAPICapture::destroy()
{
	if (initialized_)
	{
		CoUninitialize();
		initialized_ = false;
	}
}

bool WASAPICapture::adjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
		if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
		{
			pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			pEx->Samples.wValidBitsPerSample = 16;
		}
	}
	else
	{
		return false;
	}

	pwfx->wBitsPerSample = 16;
	pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
	pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
	return true;
}

bool WASAPICapture::start()
{
	Mutex::lock locker(m_mutex);
	if (!initialized_)
	{
		return false;
	}
	if (enabled_)
	{
		return true;
	}

	HRESULT hr = audio_client_->Start();
	if (FAILED(hr))
	{
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to start audio client.";
		return false;
	}

	enabled_ = true;
	worker_ = std::thread([this] {
		while (this->enabled_) {
			if (!this->capture()) {
				break;
			}
		}
	});

	return true;
}

bool WASAPICapture::stop()
{
	Mutex::lock locker(m_mutex);
	if (enabled_) {
		enabled_ = false;
    LY_ASSERT(worker_.joinable());
		worker_.join();

		HRESULT hr = audio_client_->Stop();
		if (FAILED(hr))
		{
      ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Failed to stop audio client.";
			return false;
		}
	}

	return true;
}

void WASAPICapture::setCallback(PacketCallback callback)
{
	Mutex::lock locker(m_mutex2);
	callback_ = callback;
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
    ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Faild to get next data packet size.";
		return false;
	}

	if (packetLength == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return true;
	}

	while (packetLength > 0)
	{
		hr = audio_capture_client_->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
		if (FAILED(hr))
		{
      ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Faild to get buffer.";
			return false;
		}

		if (pcm_buf_size_ < numFramesAvailable * mix_format_->nBlockAlign)
		{
			pcm_buf_size_ = numFramesAvailable * mix_format_->nBlockAlign;
			pcm_buf_.reset(new uint8_t[pcm_buf_size_], std::default_delete<uint8_t[]>());
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
		{
			memset(pcm_buf_.get(), 0, pcm_buf_size_);
		}
		else
		{
			memcpy(pcm_buf_.get(), pData, numFramesAvailable * mix_format_->nBlockAlign);
		}

		{
			Mutex::lock locker(m_mutex2);
			if (callback_) {
				callback_(mix_format_, pData, numFramesAvailable);
			}
		}

		hr = audio_capture_client_->ReleaseBuffer(numFramesAvailable);
		if (FAILED(hr))
		{
      ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Faild to release buffer.";
			return false;
		}

		hr = audio_capture_client_->GetNextPacketSize(&packetLength);
		if (FAILED(hr))
		{
      ILOG_ERROR(g_multimedia_logger) << "[WASAPICapture] Faild to get next data packet size.";
			return false;
		}
	}

	return true;
}
LY_NAMESPACE_END
#endif
