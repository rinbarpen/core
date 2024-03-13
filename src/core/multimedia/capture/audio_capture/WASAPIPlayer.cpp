#include <core/multimedia/capture/audio_capture/WASAPIPlayer.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
static auto g_multimedia_capture_logger = GET_LOGGER("multimedia.capture");

WASAPIPlayer::WASAPIPlayer() {}
WASAPIPlayer::~WASAPIPlayer() {
  this->destroy();
}

bool WASAPIPlayer::init() {
#ifdef __WIN__
  Mutex::lock locker(mutex_);
  if (initialized_) {
    return false;
  }

  CoInitialize(NULL);

  HRESULT hr = S_OK;
  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
    IID_IMMDeviceEnumerator, (void **) enumerator_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to create instance.";
    return false;
  }

  hr = enumerator_->GetDefaultAudioEndpoint(
    eRender, eMultimedia, device_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to create device.";
    return false;
  }

  hr = device_->Activate(
    IID_IAudioClient, CLSCTX_ALL, NULL, (void **) audio_client_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to activate device.";
    return false;
  }

  hr = audio_client_->GetMixFormat(&mix_format_);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to get mix format.";
    return false;
  }

  this->adjustFormatTo16Bits(mix_format_);
  hns_actual_duration_ = REFTIMES_PER_SEC;
  hr = audio_client_->Initialize(
    AUDCLNT_SHAREMODE_SHARED, 0, hns_actual_duration_, 0, mix_format_, NULL);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to initialize audio client.";
    return false;
  }

  hr = audio_client_->GetBufferSize(&buffer_frame_count_);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to get buffer size.";
    return false;
  }

  hr = audio_client_->GetService(
    IID_IAudioRenderClient, (void **) audio_render_client_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to get service.";
    return false;
  }

  // Calculate the actual duration of the allocated buffer.
  hns_actual_duration_ =
    REFTIMES_PER_SEC * buffer_frame_count_ / mix_format_->nSamplesPerSec;

  initialized_ = true;
  return true;
#else
  return false;
#endif
}
bool WASAPIPlayer::destroy() {
  return true;
}
bool WASAPIPlayer::start() {
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
      << "[WASAPIPlayer] Failed to start audio client.";
    return false;
  }

  enabled_ = true;
  worker_ = std::thread([this] {
    while (this->enabled_) {
      if (!this->play()) {
        break;
      }
    }
  });

  return true;
#else
  return false;
#endif
}
bool WASAPIPlayer::stop() {
#ifdef __WIN__
  Mutex::lock locker(mutex_);

  if (enabled_) {
    enabled_ = false;
    LY_ASSERT(worker_.joinable());
    worker_.join();

    HRESULT hr = audio_client_->Stop();
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPIPlayer] Failed to stop audio client.";
      return false;
    }
  }

  return true;
#else
  return false;
#endif
}
void WASAPIPlayer::setCallback(AudioDataCallback callback) {
  callback_ = callback;
}

bool WASAPIPlayer::adjustFormatTo16Bits(
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

bool WASAPIPlayer::play() {
#ifdef __WIN__
  uint32_t numFramesPadding = 0;
  uint32_t numFramesAvailable = 0;

  HRESULT hr = audio_client_->GetCurrentPadding(&numFramesPadding);
  if (FAILED(hr)) {
    ILOG_ERROR(g_multimedia_capture_logger)
      << "[WASAPIPlayer] Failed to get current padding.";
    return false;
  }

  numFramesAvailable = buffer_frame_count_ - numFramesPadding;

  if (numFramesAvailable > 0) {
    BYTE *data;
    hr = audio_render_client_->GetBuffer(numFramesAvailable, &data);
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPIPlayer] Audio render client failed to get buffer.";
      return false;
    }

    if (callback_) {
      ::memset(data, 0, numFramesAvailable * mix_format_->nBlockAlign);
      callback_(mix_format_, data, numFramesAvailable);
    }

    hr = audio_render_client_->ReleaseBuffer(numFramesAvailable, 0);
    if (FAILED(hr)) {
      ILOG_ERROR(g_multimedia_capture_logger)
        << "[WASAPIPlayer] Audio render client failed to release buffer.";
      return false;
    }
  }
  else {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return true;
#else
  return false;
#endif
}
LY_NAMESPACE_END
