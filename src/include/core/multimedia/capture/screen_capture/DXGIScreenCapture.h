#pragma once

#include <cstdint>
#include <vector>
#include <core/util/Mutex.h>
#include <core/util/thread/Thread.h>
#include <core/util/ds/SharedString.h>
#include <core/multimedia/capture/screen_capture/ScreenCapture.h>
#include <core/multimedia/capture/screen_capture/WASAPIHelper.h>

#ifdef __WIN__
# include <wrl.h>
# include <dxgi.h>
# include <d3d11.h>
# include <dxgi1_2.h>
#endif

LY_NAMESPACE_BEGIN
class DXGIScreenCapture : public ScreenCapture
{
	DXGIScreenCapture();
	virtual ~DXGIScreenCapture() override;

	bool init(int display_index = 0, bool auto_run = true) override;
	bool destroy() override;

	uint32_t getWidth()  const override;
	uint32_t getHeight() const override;

	bool captureFrame(std::vector<uint8_t>& bgra_image, uint32_t& width, uint32_t& height) override;
	//bool GetTextureHandle(HANDLE* handle, int* lockKey, int* unlockKey);
	//bool CaptureImage(std::string pathname);

	//ID3D11Device* GetD3D11Device() { return d3d11_device_.Get(); }
	//ID3D11DeviceContext* GetD3D11DeviceContext() { return d3d11_context_.Get(); }
  bool isCapturing() const override;

private:
	bool startCapture();
	bool stopCapture();
	bool createSharedTexture();
	bool acquireFrame();

private:
	bool initialized_{false};
	bool started_{false};
	Thread worker_{"DXGIScreenCapture"};

	Mutex::type mutex_;
  SharedString<uint8_t> bgra_image_;

#ifdef __WIN__
	DX::Monitor monitor_;
	// d3d resource
	DXGI_OUTDUPL_DESC dxgi_desc_;
	HANDLE texture_handle_{nullptr};
	int key_{0};
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_context_;
	Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dxgi_output_duplication_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shared_texture_;
	Microsoft::WRL::ComPtr<IDXGIKeyedMutex> keyed_mutex_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rgba_texture_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> gdi_texture_;
#endif
};
LY_NAMESPACE_END
