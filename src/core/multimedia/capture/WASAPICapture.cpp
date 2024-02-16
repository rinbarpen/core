#include <core/multimedia/capture/WASAPICapture.h>
#include <core/util/logger/Logger.h>
#include <memory>

LY_NAMESPACE_BEGIN
static auto g_multimedia_capture_logger = GET_LOGGER("multimedia.capture");

WASAPICapture::WASAPICapture()
  : pcm_buf_size_(48000 * 32 * 2 * 2)
  , pcm_buf_(
      new uint8_t[48000 * 32 * 2 * 2], std::default_delete<uint8_t[]>()) {}
WASAPICapture::~WASAPICapture() {}

bool WASAPICapture::init() {
#ifdef __WIN__
  Mutex::lock locker(mutex_);
  if (initialized_) return true;

  CoInitialize(NULL);

  HRESULT hr = S_OK;
  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
    IID_IMMDeviceEnumerator, (void **) enumerator_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to create instance.";
    return false;
  }

  hr = enumerator_->GetDefaultAudioEndpoint(
    eRender, eMultimedia, device_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to create device.";
    return false;
  }

  hr = device_->Activate(
    IID_IAudioClient, CLSCTX_ALL, NULL, (void **) audio_client_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to activate device.";
    return false;
  }

  hr = audio_client_->GetMixFormat(&mix_format_);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to get mix format.";
    return false;
  }

  adjustFormatTo16Bits(mix_format_);
  hns_actual_duration_ = REFTIMES_PER_SEC;
  hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK, hns_actual_duration_, 0, mix_format_, NULL);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to initialize audio client.";
    return false;
  }

  hr = audio_client_->GetBufferSize(&buffer_frame_count_);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to get buffer size.";
    return false;
  }

  hr = audio_client_->GetService(
    IID_IAudioCaptureClient, (void **) audio_capture_client_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to get service.";
    return false;
  }

  // Calculate the actual duration of the allocated buffer.
  hns_actual_duration_ = REFERENCE_TIME(REFTIMES_PER_SEC * buffer_frame_count_ / mix_format_->nSamplesPerSec);

  initialized_ = true;
  return true;
#endif
  return false;
}

bool WASAPICapture::destroy() {
#ifdef __WIN__
  if (initialized_) {
    CoUninitialize();
    initialized_ = false;
  }
  return true;
#endif
  return false;
}

bool WASAPICapture::adjustFormatTo16Bits(
#ifdef __WIN__
  WAVEFORMATEX *
#else
  void *
#endif
    pwfx) {
#ifdef __WIN__
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
#else
  return false;
#endif
}

bool WASAPICapture::start() {
#ifdef __WIN__
  Mutex::lock locker(mutex_);
  if (!initialized_) {
    return false;
  }
  if (enabled_) {
    return true;
  }

  HRESULT hr = audio_client_->Start();
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to start audio client.";
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
#else
  return false;
#endif
}

bool WASAPICapture::stop() {
#ifdef __WIN__
  Mutex::lock locker(mutex_);
  if (enabled_) {
    enabled_ = false;
    LY_ASSERT(worker_.joinable());
    worker_.join();

    HRESULT hr = audio_client_->Stop();
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPICapture] Failed to stop audio client.";
      return false;
    }
  }

  return true;
#else
  return false;
#endif
}

void WASAPICapture::setCallback(PacketCallback callback) {
  Mutex::lock locker(mutex2_);
  callback_ = callback;
}
AudioFormat WASAPICapture::getAudioFormat() const
{
#ifdef __WIN__
  AudioFormat audio_format;
  audio_format.channels = mix_format_->nChannels;
	audio_format.samples_per_sec = mix_format_->nSamplesPerSec;
	audio_format.bits_per_sample = mix_format_->wBitsPerSample;
  return audio_format;
#else
  return {};
#endif
}

bool WASAPICapture::capture() {
#ifdef __WIN__
  HRESULT hr = S_OK;
  uint32_t packetLength = 0;
  uint32_t numFramesAvailable = 0;
  BYTE *pData;
  DWORD flags;

  hr = audio_capture_client_->GetNextPacketSize(&packetLength);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPICapture] Failed to get next data packet size.";
    return false;
  }

  if (packetLength == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
  }

  while (packetLength > 0) {
    hr = audio_capture_client_->GetBuffer(
      &pData, &numFramesAvailable, &flags, NULL, NULL);
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPICapture] Failed to get buffer.";
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

    {
      Mutex::lock locker(mutex2_);
      if (callback_) {
        callback_(mix_format_, pData, numFramesAvailable);
      }
    }

    hr = audio_capture_client_->ReleaseBuffer(numFramesAvailable);
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPICapture] Failed to release buffer.";
      return false;
    }

    hr = audio_capture_client_->GetNextPacketSize(&packetLength);
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPICapture] Failed to get next data packet size.";
      return false;
    }
  }

  return true;
#else
  return false;
#endif
}
LY_NAMESPACE_END
