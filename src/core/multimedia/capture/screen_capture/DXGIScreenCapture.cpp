#include <core/multimedia/capture/screen_capture/DXGIScreenCapture.h>
#include <core/util/logger/Logger.h>
#include <fstream>

#ifdef __WIN__
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#endif

LY_NAMESPACE_BEGIN
static auto g_capture_logger = GET_LOGGER("multimedia.capture");
DXGIScreenCapture::DXGIScreenCapture() {
#ifdef __WIN__
  memset(&monitor_, 0, sizeof(DX::Monitor));
  memset(&dxgi_desc_, 0, sizeof(dxgi_desc_));
#endif
}
DXGIScreenCapture::~DXGIScreenCapture() {
  this->destroy();
}

bool DXGIScreenCapture::init(int display_index, bool auto_run) {
  if (initialized_) {
    return true;
  }

#ifdef __WIN__
  std::vector<DX::Monitor> monitors = DX::monitors();
  if (monitors.size() <= display_index) {
    return false;
  }

  monitor_ = monitors[display_index];

  HRESULT hr = S_OK;
  D3D_FEATURE_LEVEL feature_level;
  hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr,
    0, D3D11_SDK_VERSION, d3d11_device_.GetAddressOf(), &feature_level,
    d3d11_context_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create d3d11 device.";
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory;
  hr = CreateDXGIFactory1(
    __uuidof(IDXGIFactory), (void **) dxgi_factory.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create dxgi factory.";
    this->destroy();
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
  Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
  int index = 0;
  do {
    if (dxgi_factory->EnumAdapters(index, dxgi_adapter.GetAddressOf())
        != DXGI_ERROR_NOT_FOUND) {
      if (dxgi_adapter->EnumOutputs(display_index, dxgi_output.GetAddressOf())
          != DXGI_ERROR_NOT_FOUND) {
        if (dxgi_output.Get() != nullptr) {
          break;
        }
      }
    }
  } while (0);

  if (dxgi_adapter.Get() == nullptr) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] DXGI adapter not found.";
    this->destroy();
    return false;
  }

  if (dxgi_output.Get() == nullptr) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] DXGI output not found.";
    this->destroy();
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIOutput1> dxgiOutput1;
  hr = dxgi_output.Get()->QueryInterface(__uuidof(IDXGIOutput1),
    reinterpret_cast<void **>(dxgiOutput1.GetAddressOf()));
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to query interface dxgiOutput1.";
    this->destroy();
    return false;
  }

  hr = dxgiOutput1->DuplicateOutput(
    d3d11_device_.Get(), dxgi_output_duplication_.GetAddressOf());
  if (FAILED(hr)) {
    /* 0x887a0004: NVIDIA控制面板-->全局设置--首选图形处理器(自动选择) */
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to get duplicate output.";
    this->destroy();
    return false;
  }

  dxgi_output_duplication_->GetDesc(&dxgi_desc_);

  if (!this->createSharedTexture()) {
    this->destroy();
    return false;
  }
#endif

  initialized_ = true;
  this->startCapture();
  return true;
}
bool DXGIScreenCapture::destroy() {
  if (initialized_) {
    this->stopCapture();
#ifdef __WIN__
    rgba_texture_.Reset();
    gdi_texture_.Reset();
    keyed_mutex_.Reset();
    shared_texture_.Reset();
    dxgi_output_duplication_.Reset();
    d3d11_device_.Reset();
    d3d11_context_.Reset();
    memset(&dxgi_desc_, 0, sizeof(dxgi_desc_));
#endif
    initialized_ = false;
  }
  return true;
}


bool DXGIScreenCapture::captureFrame(
  std::vector<uint8_t> &bgra_image, uint32_t &width, uint32_t &height) {
  Mutex::lock locker(mutex_);

  if (!started_) {
    bgra_image.clear();
    return false;
  }

  if (bgra_image_.empty()) {
    bgra_image.clear();
    return false;
  }

  if (bgra_image.capacity() < bgra_image_.size()) {
    bgra_image.reserve(bgra_image_.size());
  }

  bgra_image.assign(bgra_image_.get(), bgra_image_.get() + bgra_image_.size());
#ifdef __WIN__
  width = dxgi_desc_.ModeDesc.Width;
  height = dxgi_desc_.ModeDesc.Height;
#endif
  return true;
}

uint32_t DXGIScreenCapture::getWidth() const {
#ifdef __WIN__
  return dxgi_desc_.ModeDesc.Width;
#else
  return 0;
#endif
}
uint32_t DXGIScreenCapture::getHeight() const {
#ifdef __WIN__
  return dxgi_desc_.ModeDesc.Height;
#else
  return 0;
#endif
}
bool DXGIScreenCapture::isCapturing() const {
  return started_;
}

bool DXGIScreenCapture::startCapture() {
  if (!initialized_) {
    return false;
  }
  if (started_) {
    return true;
  }

  started_ = true;
  this->acquireFrame();
  worker_.dispatch([this] {
    while (started_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      this->acquireFrame();
    }
  });

  return true;
}
bool DXGIScreenCapture::stopCapture() {
  started_ = false;
  worker_.destroy();
  return true;
}
bool DXGIScreenCapture::createSharedTexture() {
#ifdef __WIN__
  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = dxgi_desc_.ModeDesc.Width;
  desc.Height = dxgi_desc_.ModeDesc.Height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.BindFlags = 0;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags =
    D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;  // D3D11_RESOURCE_MISC_SHARED;

  HRESULT hr = d3d11_device_->CreateTexture2D(
    &desc, nullptr, shared_texture_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create texture.";
    return false;
  }

  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;

  hr = d3d11_device_->CreateTexture2D(
    &desc, nullptr, rgba_texture_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create texture.";
    return false;
  }

  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET;
  desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

  hr =
    d3d11_device_->CreateTexture2D(&desc, nullptr, gdi_texture_.GetAddressOf());
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create texture.";
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIResource> dxgiResource;
  hr = shared_texture_->QueryInterface(__uuidof(IDXGIResource),
    reinterpret_cast<void **>(dxgiResource.GetAddressOf()));
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger) << "[DXGIScreenCapture] Failed to query "
                                    "IDXGIResource interface from texture.";
    return false;
  }

  hr = dxgiResource->GetSharedHandle(&texture_handle_);
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to get shared handle.";
    return false;
  }

  hr = shared_texture_->QueryInterface(_uuidof(IDXGIKeyedMutex),
    reinterpret_cast<void **>(keyed_mutex_.GetAddressOf()));
  if (FAILED(hr)) {
    ILOG_ERROR(g_capture_logger)
      << "[DXGIScreenCapture] Failed to create key mutex.";
    return false;
  }
#endif
  return true;
}
bool DXGIScreenCapture::acquireFrame() {
#ifdef __WIN__
  Microsoft::WRL::ComPtr<IDXGIResource> dxgi_resource;
  DXGI_OUTDUPL_FRAME_INFO frame_info;
  memset(&frame_info, 0, sizeof(DXGI_OUTDUPL_FRAME_INFO));

  dxgi_output_duplication_->ReleaseFrame();
  HRESULT hr = dxgi_output_duplication_->AcquireNextFrame(
    0, &frame_info, dxgi_resource.GetAddressOf());

  if (FAILED(hr)) {
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
      return false;
    }
    if (hr == DXGI_ERROR_INVALID_CALL) {
      return false;
    }
    if (hr == DXGI_ERROR_ACCESS_LOST) {
      return false;
    }

    return false;
  }

  if (frame_info.AccumulatedFrames == 0
      || frame_info.LastPresentTime.QuadPart == 0) {
    // No image update, only cursor moved.
  }

  if (!dxgi_resource.Get()) {
    return false;
  }

  Microsoft::WRL::ComPtr<ID3D11Texture2D> outputTexture;
  hr = dxgi_resource->QueryInterface(__uuidof(ID3D11Texture2D),
    reinterpret_cast<void **>(outputTexture.GetAddressOf()));
  if (FAILED(hr)) {
    return false;
  }

  Mutex::lock locker(mutex_);
  bgra_image_.resize(
    dxgi_desc_.ModeDesc.Width * dxgi_desc_.ModeDesc.Height * 4);

  D3D11_MAPPED_SUBRESOURCE dsec = {0};
  d3d11_context_->CopyResource(gdi_texture_.Get(), outputTexture.Get());

  Microsoft::WRL::ComPtr<IDXGISurface1> surface1;
  hr = gdi_texture_->QueryInterface(__uuidof(IDXGISurface1),
    reinterpret_cast<void **>(surface1.GetAddressOf()));
  if (FAILED(hr)) {
    return false;
  }

  CURSORINFO cursorInfo = {0};
  cursorInfo.cbSize = sizeof(CURSORINFO);
  if (GetCursorInfo(&cursorInfo) == TRUE) {
    if (cursorInfo.flags == CURSOR_SHOWING) {
      auto cursorPosition = cursorInfo.ptScreenPos;
      auto lCursorSize = cursorInfo.cbSize;
      HDC hdc;
      surface1->GetDC(FALSE, &hdc);
      DrawIconEx(hdc, cursorPosition.x - monitor_.left,
        cursorPosition.y - monitor_.top, cursorInfo.hCursor, 0, 0, 0, 0,
        DI_NORMAL | DI_DEFAULTSIZE);
      surface1->ReleaseDC(nullptr);
    }
  }

  d3d11_context_->CopyResource(rgba_texture_.Get(), gdi_texture_.Get());
  hr = d3d11_context_->Map(rgba_texture_.Get(), 0, D3D11_MAP_READ, 0, &dsec);
  if (!FAILED(hr)) {
    if (dsec.pData != NULL) {
      uint32_t image_width = this->getWidth();
      uint32_t image_height = this->getHeight();
      bgra_image_.reset();

      for (uint32_t y = 0; y < image_height; y++) {
        memcpy(bgra_image_.get() + y * image_width * 4,
          (uint8_t *) dsec.pData + y * dsec.RowPitch, image_width * 4);
      }
    }
    d3d11_context_->Unmap(rgba_texture_.Get(), 0);
  }

  hr = keyed_mutex_->AcquireSync(0, 5);
  if (hr != S_OK) {
    return true;
  }
  d3d11_context_->CopyResource(shared_texture_.Get(), rgba_texture_.Get());
  keyed_mutex_->ReleaseSync(key_);
#endif
  return true;
}

LY_NAMESPACE_END
