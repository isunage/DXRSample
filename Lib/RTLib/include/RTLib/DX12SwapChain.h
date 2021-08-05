#ifndef RTLIB_DX12_SWAPCHAIN_H
#define RTLIB_DX12_SWAPCHAIN_H
#include "DX12.h"
#include "DX12Context.h"
namespace rtlib{
    class DX12SwapChain {
    public:
        static inline constexpr UINT kMaxFrameCount = 3;
    public:
        DX12SwapChain(
            DX12Context* context, HWND hwnd,
            UINT width, UINT height, 
            UINT backBufferCount,UINT flags = 0,
            DXGI_FORMAT backBufferFormat   = DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT):  m_Context{context},
            m_Hwnd{hwnd}, 
            m_Width{width},
            m_Height{height},
            m_BackBufferCount{backBufferCount},
            m_BackBufferFormat{ backBufferFormat }, 
            m_DepthStencilFormat{ depthStencilFormat }, 
            m_FenceValues{},
            m_Flags{ flags }, 
            m_BackBufferIndex{0},
            m_RtvDescriptorSize{0},
            m_DsvDescriptorSize{0} {}

        virtual void OnInit() {
            InitSwapChain();
            InitBackBuffers();
            InitDepthStencil();
            InitCommands();
            InitFence();
        }

        virtual void OnDestroy() {
            for (UINT n = 0; n < m_BackBufferCount; ++n)
            {
                m_CommandAllocators[n].Reset();
                m_BackBuffers[n].Reset();
            }

            m_DepthStencil.Reset();
            m_CommandList.Reset();
            
            m_Fence.Reset();
            m_RtvDescriptorHeap.Reset();
            m_DsvDescriptorHeap.Reset();
            m_SwapChain.Reset();
        }
        
        void BeginFrame(D3D12_RESOURCE_STATES stateBefore = D3D12_RESOURCE_STATE_PRESENT);
        bool   EndFrame(D3D12_RESOURCE_STATES stateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void ExecuteCommandList();
        void WaitForGPU();
        void MoveToNextFrame();

        auto GetSwapChain ()const -> IDXGISwapChain* {return m_SwapChain.Get(); }
        auto GetHwnd()const -> HWND  { return m_Hwnd;  }
        auto GetWidth()const -> UINT { return m_Width; }
        auto GetHeight()const-> UINT { return m_Height;}
        //BackBuffers
        auto GetBackBuffer()const -> ID3D12Resource* {return m_BackBuffers[m_BackBufferIndex].Get();}
        auto GetBackBufferIndex()const -> UINT { return m_BackBufferIndex; }
        auto GetBackBufferCount()const -> UINT { return m_BackBufferCount; }
        auto GetBackBufferFormat()const -> DXGI_FORMAT { return m_BackBufferFormat; }
        auto GetRtvDescriptorHeap()const -> ID3D12DescriptorHeap* { return m_RtvDescriptorHeap.Get();}
        auto GetRtvh()const ->CD3DX12_CPU_DESCRIPTOR_HANDLE {
            auto rtvh = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_BackBufferIndex, m_RtvDescriptorSize);
            return rtvh;
        }
        //DepthStencil
        auto GetDepthStencil()const -> ID3D12Resource* { return m_DepthStencil.Get();}
        auto GetDepthStencilFormat()const ->DXGI_FORMAT {return m_DepthStencilFormat; }
        auto GetDsvDescriptorHeap()const -> ID3D12DescriptorHeap* { return m_DsvDescriptorHeap.Get();}
        auto GetDsvh()const ->CD3DX12_CPU_DESCRIPTOR_HANDLE {
            auto dsvh = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
            return dsvh;
        }
        //Command Allocator
        auto GetCommandAllocator()const -> ID3D12CommandAllocator* { return m_CommandAllocators[m_BackBufferIndex].Get();}
        auto GetCommandList()const -> ID3D12GraphicsCommandList* { return m_CommandList.Get();}
        //Fence
        auto GetFence() const -> ID3D12Fence* { return m_Fence.Get(); }
        auto GetFenceValue() const -> UINT64  { return m_FenceValues[m_BackBufferIndex];}
        auto GetEvent() const -> HANDLE { return m_FenceEvent.Get(); }
    protected:
        virtual void InitSwapChain();
        virtual void InitBackBuffers();
        virtual void InitDepthStencil();
        virtual void InitCommands();
        virtual void InitFence();
    protected:
        DX12Context*                      m_Context = nullptr;
        ComPtr<IDXGISwapChain4>           m_SwapChain; 
        UINT                              m_Flags;
        HWND                              m_Hwnd     = nullptr;
        UINT                              m_Width;
        UINT                              m_Height;
        D3D12_VIEWPORT                    m_Viewport = {};
        D3D12_RECT                        m_Scissor  = {};

        ComPtr<ID3D12Resource>            m_BackBuffers[kMaxFrameCount];
        UINT                              m_BackBufferIndex;
        UINT                              m_BackBufferCount;
        DXGI_FORMAT                       m_BackBufferFormat;
        ComPtr<ID3D12DescriptorHeap>      m_RtvDescriptorHeap;
        UINT                              m_RtvDescriptorSize;

        ComPtr<ID3D12Resource>            m_DepthStencil;
        DXGI_FORMAT                       m_DepthStencilFormat;
        ComPtr<ID3D12DescriptorHeap>      m_DsvDescriptorHeap;
        UINT                              m_DsvDescriptorSize;

        ComPtr<ID3D12CommandAllocator>    m_CommandAllocators[kMaxFrameCount];
        ComPtr<ID3D12GraphicsCommandList> m_CommandList;
        
        ComPtr<ID3D12Fence>               m_Fence;
        UINT64                            m_FenceValues[kMaxFrameCount];
        Wrappers::Event                   m_FenceEvent;
    };
}
#endif