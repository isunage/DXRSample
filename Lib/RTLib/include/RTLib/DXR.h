#ifndef RTLIB_DXR_H
#define RTLIB_DXR_H
#include "DX12.h"
namespace rtlib {
    inline bool IsDXRSupported(IDXGIAdapter1* adapter)
    {
        ComPtr<ID3D12Device> testDevice;
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 dxrFeatures;
        return SUCCEEDED(D3D12CreateDevice(adapter,D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
            && SUCCEEDED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &dxrFeatures, sizeof(dxrFeatures)))
            && dxrFeatures.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }
}
#endif