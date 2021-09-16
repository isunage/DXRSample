#ifndef RTLIB_DX12_CONTEXT_H
#define RTLIB_DX12_CONTEXT_H
#include "DX12.h"
#include <string>
namespace rtlib
{
	class DX12Context {
	public:
		struct TearingFlags {
			static inline constexpr UINT eNone     = 0x0;
			static inline constexpr UINT eAllow    = 0x1;
			static inline constexpr UINT eRequired = 0x2;
		};
	public:
		DX12Context(D3D_FEATURE_LEVEL featLevel = D3D_FEATURE_LEVEL_11_0, UINT options = 0) :m_FeatLevel(featLevel), m_Options{ options }, m_AdapterID{UINT_MAX}{
			if (m_Options & TearingFlags::eRequired)
			{
				m_Options |= TearingFlags::eAllow;
			}
		}
		~DX12Context() {}

		void OnInit() {
			InitFactory();
			InitAdapter();
			InitDevice();
			InitGCmdQueue();
			InitCCmdQueue();
			InitTCmdQueue();
		}
		void OnDeviceLost() {
			OnDestroy();
			InitFactory();
			InitAdapter();
			InitDevice();
			InitGCmdQueue();
			InitTCmdQueue();
			InitCCmdQueue();
		}
		void ShowRefCount() {
			if (m_GCommandQueue) {
				m_GCommandQueue->AddRef();
				fmt::print("GCmdQueue RefCnt: {}\n", m_GCommandQueue->Release());
			}
			if (m_TCommandQueue) {
				m_TCommandQueue->AddRef();
				fmt::print("TCmdQueue RefCnt: {}\n", m_TCommandQueue->Release());
			}
			if (m_CCommandQueue) {
				m_CCommandQueue->AddRef();
				fmt::print("CCmdQueue RefCnt: {}\n", m_CCommandQueue->Release());
			}
			if (m_Device) {
				m_Device->AddRef();
				fmt::print("   Device RefCnt: {}\n", m_Device->Release());
			}
			if (m_Adapter) {
				m_Adapter->AddRef();
				fmt::print("  Adapter RefCnt: {}\n", m_Adapter->Release());
			}
			if (m_Factory) {
				m_Factory->AddRef();
				fmt::print("  Factory RefCnt: {}\n", m_Factory->Release());
			}
		}
		void OnDestroy() {
			m_GCommandQueue.Reset();
			m_TCommandQueue.Reset();
			m_CCommandQueue.Reset();
			m_Device.Reset();
			m_Adapter.Reset();
			m_Factory.Reset();
		}

		auto GetFeatLevel()const -> D3D_FEATURE_LEVEL   { return m_FeatLevel;     }
		auto GetFactory()const   -> IDXGIFactory4*      { return m_Factory.Get(); }
		auto GetAdapter()const   -> IDXGIAdapter*       { return m_Adapter.Get(); }
		auto GetDevice() const   -> ID3D12Device*       { return m_Device.Get();  }
		auto GetGCmdQueue()const -> ID3D12CommandQueue* { return m_GCommandQueue.Get(); }
		auto GetTCmdQueue()const -> ID3D12CommandQueue* { return m_TCommandQueue.Get(); }
		auto GetCCmdQueue()const -> ID3D12CommandQueue* { return m_CCommandQueue.Get(); }

		bool SupportDXR()const;
	private:
		//Init
		void InitFactory();
		void InitAdapter();

		void InitDevice();
		void InitGCmdQueue();
		void InitCCmdQueue();
		void InitTCmdQueue();
		//Destroy

	private:
		D3D_FEATURE_LEVEL          m_FeatLevel;
		ComPtr<IDXGIFactory4>	   m_Factory;
		ComPtr<IDXGIAdapter>       m_Adapter;
		ComPtr<ID3D12Device>       m_Device;
		ComPtr<ID3D12CommandQueue> m_GCommandQueue;
		ComPtr<ID3D12CommandQueue> m_TCommandQueue;
		ComPtr<ID3D12CommandQueue> m_CCommandQueue;
		UINT                       m_Options;
		UINT                       m_AdapterID;
	};
}
#endif