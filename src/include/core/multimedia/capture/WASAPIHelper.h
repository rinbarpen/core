#pragma once

#include "core/util/marcos.h"
#ifdef __WIN__
#include <vector>
#include <d3d9.h>

LY_NAMESPACE_BEGIN

namespace DX
{
struct Monitor
{
	uint64_t low_part;
	uint64_t high_part;

	int left;
	int top;
	int right;
	int bottom;
};

static auto getMonitors() -> std::vector<Monitor>
{
	std::vector<Monitor> monitors;
	HRESULT hr = S_OK;

	IDirect3D9Ex* d3d9ex = nullptr;
	hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9ex);
	if (FAILED(hr)) {
		return monitors;
	}

	int adapter_count = d3d9ex->GetAdapterCount();
  monitors.reserve(adapter_count);
	for (int i = 0; i < adapter_count; i++) {
		Monitor monitor;
		memset(&monitor, 0, sizeof(Monitor));

		LUID luid = { 0 , 0 };
		hr = d3d9ex->GetAdapterLUID(i, &luid);
		if (FAILED(hr)) {
			continue;
		}

		monitor.low_part = (uint64_t)luid.LowPart;
		monitor.high_part = (uint64_t)luid.HighPart;

		HMONITOR hMonitor = d3d9ex->GetAdapterMonitor(i);
		if (hMonitor) {
			MONITORINFO monitor_info;
			monitor_info.cbSize = sizeof(MONITORINFO);
			BOOL ret = GetMonitorInfoA(hMonitor, &monitor_info);
			if (ret) {
				monitor.left = monitor_info.rcMonitor.left;
				monitor.right = monitor_info.rcMonitor.right;
				monitor.top = monitor_info.rcMonitor.top;
				monitor.bottom = monitor_info.rcMonitor.bottom;
				monitors.push_back(monitor);
			}
		}
	}

	d3d9ex->Release();
	return monitors;
}

}
LY_NAMESPACE_END
#endif
