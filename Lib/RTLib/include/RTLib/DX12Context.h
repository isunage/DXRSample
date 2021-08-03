#ifndef RTLIB_DX12_CONTEXT_H
#define RTLIB_DX12_CONTEXT_H
#include "DX12.h"
#include <string>
namespace rtlib {
    class DX12DeviceNotify 
    {
    public:
        virtual void OnDeviceLost()     = 0;
        virtual void OnDeviceRestored() = 0;
        virtual ~DX12DeviceNotify(){}
    };
    class DX12Context {
        DX12Context(){}
    public:
        inline static constexpr unsigned int kAllowTearing          = 0x1;
        inline static constexpr unsigned int kRequireTearingSupport = 0x2;
        inline static constexpr std::size_t  kMaxBackBufferCount    = 3;
    public:
        
        DX12Context(
            DXGI_FORMAT       backBufferFormat  = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT       depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT              backBufferCount   = 2,
            D3D_FEATURE_LEVEL minFeatureLevel   = D3D_FEATURE_LEVEL_11_0,
            UINT              flags             = 0,
            UINT              adapterIdOverride = UINT_MAX
        );
        ~DX12Context();
        
        void InitializeDXGIAdapter();
        void SetAdapterOverride(UINT adapterID){ m_AdapterIdOverride = adapterID; }
        void CreateDeviceResources();
        void CreateWindowSizeDependentResource();
        void SetWindow(HWND window, int width, int height);
        void HandleDeviceLost();
        void RegisterDeviceNotify(DX12DeviceNotify* deviceNotify)
        {
            m_DeviceNotify = deviceNotify;
            __if_exists(DXGIDeclareAdapterRemovalSupport) {
                if (deviceNotify) {
                    if (FAILED(DXGIDeclareAdapterRemovalSupport()))
                    {

                    }
                }
            }
        }
        void Prepare(D3D12_RESOURCE_STATES stateBefore = D3D12_RESOURCE_STATE_PRESENT);
        void Present(D3D12_RESOURCE_STATES stateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void ExecuteCommandList();
        void WaitForGPU();

        RECT GetOutputSize()const    { return m_OutputSize; }
        bool IsWindowVisible()const  { return m_IsWindowVisible; }
        bool IsTearingSupport()const { return m_Options & kAllowTearing; }
        auto GetAdapter()const -> IDXGIAdapter1* { return m_Adapter.Get();  }
        auto GetDevice()const  -> ID3D12Device*  { return m_Device.Get(); }
        auto GetFactory()const -> IDXGIFactory4* { return m_Factory.Get(); }
        auto GetSwapChain()const -> IDXGISwapChain* { return m_SwapChain.Get(); }
        auto GetDeviceFeatureLevel()const -> D3D_FEATURE_LEVEL { return m_D3dFeatureLevel; }
        auto GetRenderTarget()const -> ID3D12Resource* { return m_RenderTargets[m_BackBufferIndex].Get(); }
        auto GetDepthStencil()const -> ID3D12Resource* { return m_DepthStencil.Get(); }
        auto GetCommandQueue()const -> ID3D12CommandQueue* { return m_CommandQueue.Get(); }
        auto GetCommandAllocator() const -> ID3D12CommandAllocator* { return m_CommandAllocators[m_BackBufferIndex].Get(); }
        auto GetCommandList()const  -> ID3D12GraphicsCommandList* { return m_CommandList.Get(); }
        auto GetBackBufferFormat()const -> DXGI_FORMAT { return m_BackBufferFormat; }
        auto GetDepthBufferFormat()const -> DXGI_FORMAT { return m_DepthBufferFormat; }
        auto GetScreenViewport()const -> D3D12_VIEWPORT { return m_Viewport; }
        auto GetScissorRect()const -> D3D12_RECT { return m_Scissor; }
        auto GetCurrentFrameIndex()const -> UINT { return m_BackBufferIndex; }
        auto GetPreviousFrameIndex()const -> UINT { return m_BackBufferIndex == 0 ? m_BackBufferCount - 1 : m_BackBufferIndex - 1; }
        auto GetBackBufferCount()const -> UINT { return m_BackBufferCount; }
        auto GetDeviceOptions()const -> unsigned int { return m_Options; }
        auto GetAdapterDescription()const -> LPCWSTR { return m_AdapterDescription.c_str(); }
        auto GetAdapterID()const { return m_AdapterID; }
    private:
        void MoveToNextFrame();
        void InitializeAdapter(IDXGIAdapter1** ppAdapter);
    private:
        UINT                                    m_AdapterIdOverride;
        UINT                                    m_BackBufferIndex;
        ComPtr<IDXGIAdapter1>                   m_Adapter;
        UINT                                    m_AdapterID;
        std::wstring                            m_AdapterDescription;
        ComPtr<ID3D12Device>                    m_Device;

        ComPtr<ID3D12CommandQueue>              m_CommandQueue;
        ComPtr<ID3D12GraphicsCommandList>       m_CommandList;
        ComPtr<ID3D12CommandAllocator>          m_CommandAllocators[kMaxBackBufferCount];
        
        ComPtr<IDXGIFactory4>                   m_Factory;
        ComPtr<IDXGISwapChain3>                 m_SwapChain;
        ComPtr<ID3D12Resource>                  m_RenderTargets[kMaxBackBufferCount];
        ComPtr<ID3D12Resource>                  m_DepthStencil;

        ComPtr<ID3D12Fence>                     m_Fence;
        UINT64                                  m_FenceValues[kMaxBackBufferCount];
        Wrappers::Event                         m_FenceEvent;

        ComPtr<ID3D12DescriptorHeap>            m_RtvDescriptorHeap;
        ComPtr<ID3D12DescriptorHeap>            m_DsvDescriptorHeap;
        UINT                                    m_RtvDescriptorSize;

        D3D12_VIEWPORT                          m_Viewport;
        D3D12_RECT                              m_Scissor;

        DXGI_FORMAT                             m_BackBufferFormat;
        DXGI_FORMAT                             m_DepthBufferFormat;
        UINT                                    m_BackBufferCount;
        D3D_FEATURE_LEVEL                       m_D3dMinFeatureLevel;

        HWND                                    m_Hwnd;
        D3D_FEATURE_LEVEL                       m_D3dFeatureLevel;
        RECT                                    m_OutputSize;
        bool                                    m_IsWindowVisible;

        unsigned int                            m_Options;
        DX12DeviceNotify*                       m_DeviceNotify;
    };
}
#endif