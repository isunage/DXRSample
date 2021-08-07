#include "..\include\RTLib\DX12Context.h"
#include <fmt/format.h>
#include <cassert>

void rtlib::DX12Context::InitFactory()
{
	bool dxgiDebug = false;
#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debug;
		RTLIB_IF_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))
		{
			debug->EnableDebugLayer();
		}
		else {
			fmt::print("WARNING: Direct3D Debug Device is not available\n");
		}
		ComPtr<IDXGIInfoQueue> infoQueue;
		RTLIB_IF_SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&infoQueue))) {
			ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory)));
			infoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR     , true);
			infoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			dxgiDebug = true;
		}
	}
#endif
	if (!dxgiDebug)
	{
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));
	}
#ifdef _DEBUG
	{
		ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_SUMMARY));
		}
	}
#endif
	if (m_Options & (TearingFlags::eAllow | TearingFlags::eRequired)) {
		BOOL allowTearing = false;
		ComPtr<IDXGIFactory5> factory5;
		HRESULT hr = m_Factory.As(&factory5);
		RTLIB_IF_SUCCEEDED(hr)
		{
			hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
		if (FAILED(hr) || !allowTearing) {
			fmt::print("WARNING: Variable refresh rate displays are not supported.\n");
			if (m_Options & TearingFlags::eRequired) {
				ThrowIfFailed(S_FALSE, "Error: Sample must be run on an OS with tearing support.\n");
			}
			m_Options &= ~TearingFlags::eAllow;
		}
	}
}

void rtlib::DX12Context::InitAdapter()
{
	ComPtr<IDXGIAdapter1> adapter1;
	ComPtr<IDXGIFactory6> factory6;
	
	HRESULT hr = m_Factory.As(&factory6);

	RTLIB_IF_FAILED(hr) {
		throw std::exception("DXGI 1.6 Not Supported!");
	}

	for (auto adapterID = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterID, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter1)); ++adapterID)
	{
		if (m_AdapterID != UINT_MAX && adapterID != m_AdapterID) {
			continue;
		}

		DXGI_ADAPTER_DESC1 desc;
		ThrowIfFailed(adapter1->GetDesc1(&desc));

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}

		RTLIB_IF_SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) {
			m_AdapterID = adapterID;
			break;
		}
	}
	if (!adapter1)
	{
		if (m_AdapterID != UINT_MAX)
		{
			throw std::exception("Unavailable adapter requested!");
		}
		else {
			throw std::exception("Unavailable adapter!");
		}
	}
	adapter1.As(&m_Adapter);
}

void rtlib::DX12Context::InitDevice()
{
	ThrowIfFailed(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
#ifndef NDEBUG
	{
		ComPtr<ID3D12InfoQueue> infoQueue;
		RTLIB_IF_SUCCEEDED(m_Device.As(&infoQueue))
		{
#ifdef _DEBUG
			{
				D3D12_MESSAGE_ID hides[] = { D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE };

				D3D12_INFO_QUEUE_FILTER filter = {};

				filter.DenyList.NumIDs  = std::size(hides);
				filter.DenyList.pIDList = std::data(hides);

				infoQueue->AddStorageFilterEntries(&filter);
			}
#endif

		}
	}
#endif

	constexpr D3D_FEATURE_LEVEL s_FeatureLevels []= {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = {
		std::size(s_FeatureLevels),std::data(s_FeatureLevels),D3D_FEATURE_LEVEL_11_0
	};

	HRESULT hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));

	RTLIB_IF_SUCCEEDED(hr) {
		m_FeatLevel = featLevels.MaxSupportedFeatureLevel;
	}
	else {
		m_FeatLevel = D3D_FEATURE_LEVEL_11_0;
	}
}

bool rtlib::DX12Context::SupportDXR() const
{
	ComPtr<ID3D12Device6> device6;
	m_Device.As(&device6);

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
	RTLIB_IF_SUCCEEDED(device6->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)))
	{
		return options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	}
	return false;
}

void rtlib::DX12Context::InitGCmdQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GCommandQueue)));
	RTLIB_DX12_NAME(rtlib::DX12Context::m_GCommandQueue);
}

void rtlib::DX12Context::InitCCmdQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CCommandQueue)));
	RTLIB_DX12_NAME(rtlib::DX12Context::m_CCommandQueue);
}

void rtlib::DX12Context::InitTCmdQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_TCommandQueue)));
	RTLIB_DX12_NAME(rtlib::DX12Context::m_TCommandQueue);
}

