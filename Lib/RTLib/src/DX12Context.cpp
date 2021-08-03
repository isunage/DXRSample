#include "..\include\RTLib\DX12Context.h"
#include <cassert>

void rtlib::DX12Context::Prepare(D3D12_RESOURCE_STATES stateBefore)
{
	ThrowIfFailed(m_CommandAllocators[m_BackBufferIndex]->Reset());
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocators[m_BackBufferIndex].Get(), nullptr));
	if (stateBefore != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTarget(), stateBefore, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
}

void rtlib::DX12Context::Present(D3D12_RESOURCE_STATES stateBefore)
{
	if (stateBefore != D3D12_RESOURCE_STATE_PRESENT) {
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTarget(), stateBefore, D3D12_RESOURCE_STATE_PRESENT));
	}
	ExecuteCommandList();
	HRESULT hr;
	if (m_Options & kAllowTearing)
	{
		hr = m_SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}
	else {
		hr = m_SwapChain->Present(1, 0);
	}
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
	}
	else {
		ThrowIfFailed(hr);
		MoveToNextFrame();
	}
}

void rtlib::DX12Context::MoveToNextFrame()
{
}

void rtlib::DX12Context::InitializeAdapter(IDXGIAdapter1** ppAdapter)
{
}

rtlib::DX12Context::DX12Context(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel, UINT flags, UINT adapterIdOverride):
	m_BackBufferIndex(0),
	m_FenceValues{},
	m_RtvDescriptorSize(0),
	m_Viewport{}, m_Scissor{},
	m_BackBufferFormat{backBufferFormat},
	m_DepthBufferFormat{depthBufferFormat},
	m_D3dMinFeatureLevel{D3D_FEATURE_LEVEL_11_0},
	m_OutputSize{ 0,0,1,1 },
	m_Options{flags},
	m_IsWindowVisible(true),
	m_AdapterIdOverride(adapterIdOverride),
	m_AdapterID(UINT_MAX)
{
	assert(backBufferCount <= kMaxBackBufferCount);
	assert(minFeatureLevel <= D3D_FEATURE_LEVEL_11_0);

	if (m_Options & kRequireTearingSupport) {
		m_Options |= kAllowTearing;
	}
}

rtlib::DX12Context::~DX12Context()
{
	WaitForGPU();
}

void rtlib::DX12Context::InitializeDXGIAdapter()
{
	bool debugDXGI = false;

}
