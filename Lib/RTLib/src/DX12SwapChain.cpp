#include "../include/RTLib/DX12SwapChain.h"

void rtlib::DX12SwapChain::BeginFrame(D3D12_RESOURCE_STATES stateBefore) {
    m_CommandAllocators[m_BackBufferIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[m_BackBufferIndex].Get(), nullptr);
    if (stateBefore != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        m_CommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                m_BackBuffers[m_BackBufferIndex].Get(),
                stateBefore,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            )
        );
    }
}

bool rtlib::DX12SwapChain::EndFrame(D3D12_RESOURCE_STATES stateBefore) {
    if (stateBefore != D3D12_RESOURCE_STATE_PRESENT) {
        m_CommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                m_BackBuffers[m_BackBufferIndex].Get(),
                stateBefore,
                D3D12_RESOURCE_STATE_PRESENT
            )
        );
    }

    ExecuteCommandList();

    HRESULT hr = m_SwapChain->Present(1, 0);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        return false;
    }
    ThrowIfFailed(hr);
    MoveToNextFrame();
    return true;
}

void rtlib::DX12SwapChain::ExecuteCommandList() {
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* commandLists[] = { m_CommandList.Get() };
    m_Context->GetGCmdQueue()->ExecuteCommandLists(1, commandLists);
}

void rtlib::DX12SwapChain::WaitForGPU() {
    if (m_Context && m_Context->GetGCmdQueue() && m_FenceEvent.IsValid()) {
        RTLIB_IF_SUCCEEDED(m_Context->GetGCmdQueue()->Signal(m_Fence.Get(), m_FenceValues[m_BackBufferIndex]))
        {

            if (m_Fence->GetCompletedValue() < m_FenceValues[m_BackBufferIndex]) {
                m_Fence->SetEventOnCompletion(m_FenceValues[m_BackBufferIndex], m_FenceEvent.Get());
                WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
                m_FenceValues[m_BackBufferIndex]++;
            }
        }
    }
}

void rtlib::DX12SwapChain::MoveToNextFrame()
{

    const UINT64 currentFenceValue = m_FenceValues[m_BackBufferIndex];
    ThrowIfFailed(m_Context->GetGCmdQueue()->Signal(m_Fence.Get(), currentFenceValue));

    m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    if (m_Fence->GetCompletedValue() < m_FenceValues[m_BackBufferIndex])
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_BackBufferIndex], m_FenceEvent.Get()));
        WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
    }

    m_FenceValues[m_BackBufferIndex] = currentFenceValue + 1;
}

void rtlib::DX12SwapChain::InitSwapChain() {
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
    swapChainDesc.Format = m_BackBufferFormat;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_BackBufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = m_Flags;
    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_Context->GetFactory()->CreateSwapChainForHwnd(m_Context->GetGCmdQueue(), m_Hwnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()));
    ThrowIfFailed(swapChain.As(&m_SwapChain));
    m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void rtlib::DX12SwapChain::InitBackBuffers() {
    m_RtvDescriptorSize = m_Context->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = m_BackBufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Context->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvDescriptorHeap)));
    RTLIB_DX12_NAME(rtlib::DX12SwapChain::m_RtvDescriptorHeap);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = m_BackBufferFormat;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (auto i = 0; i < m_BackBufferCount; ++i) {

        RTLIB_IF_FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]))) {
            throw std::runtime_error("Failed To Get BackBuffer!");
        }
        RTLIB_DX12_NAME_INDEXED(rtlib::DX12SwapChain::m_BackBuffers, i);

        m_Context->GetDevice()->CreateRenderTargetView(m_BackBuffers[i].Get(), &rtvDesc, rtvHandle);

        rtvHandle.Offset(1, m_RtvDescriptorSize);
    }
}

void rtlib::DX12SwapChain::InitDepthStencil() {
    D3D12_CLEAR_VALUE dsClearValue = {};
    dsClearValue.Format = m_DepthStencilFormat;
    dsClearValue.DepthStencil.Depth = 1.0f;
    dsClearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(
        m_Context->GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(
                m_DepthStencilFormat, m_Width, m_Height, 1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &dsClearValue,
            IID_PPV_ARGS(&m_DepthStencil)
        )
    );
    RTLIB_DX12_NAME(rtlib::DX12SwapChain::m_DepthStencil);

    m_DsvDescriptorSize = m_Context->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Context->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvDescriptorHeap)));
    RTLIB_DX12_NAME(rtlib::DX12SwapChain::m_RtvDescriptorHeap);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_DepthStencilFormat;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;

    m_Context->GetDevice()->CreateDepthStencilView(m_DepthStencil.Get(), &dsvDesc, m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void rtlib::DX12SwapChain::InitCommands() {

    for (auto i = 0; i < m_BackBufferCount; ++i) {
        ThrowIfFailed(
            m_Context->GetDevice()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i])
            )
        );
        RTLIB_DX12_NAME_INDEXED(rtlib::DX12SwapChain::m_CommandAllocators, i);
    }

    ThrowIfFailed(
        m_Context->GetDevice()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_BackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&m_CommandList)
        )
    );
    ThrowIfFailed(m_CommandList->Close());

    RTLIB_DX12_NAME(rtlib::DX12SwapChain::m_CommandList);
}

void rtlib::DX12SwapChain::InitFence() {
    for (auto i = 0; i < m_BackBufferIndex; ++i)
    {
        m_FenceValues[i] = 0;
    }

    ThrowIfFailed(
        m_Context->GetDevice()->CreateFence(m_FenceValues[m_BackBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))
    );
    RTLIB_DX12_NAME(rtlib::DX12SwapChain::m_Fence);

    m_FenceValues[m_BackBufferIndex]++;

    m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));

    if (!m_FenceEvent.IsValid()) {
        throw std::runtime_error("Failed To Create Valid Fence Event");
    }
}
