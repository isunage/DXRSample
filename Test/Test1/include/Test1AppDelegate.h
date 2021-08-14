#ifndef TEST1_APP_DELEGATE_H
#define TEST1_APP_DELEGATE_H
#include <Win32AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
#include <RTLib/DXRAccelerationStructure.h>
namespace test {
	class Test1AppDelegate:public Win32AppDelegate
	{
	public:
        Test1AppDelegate();
        Test1AppDelegate(int width, int height, std::string title);
        static  auto New()->std::shared_ptr<Win32AppDelegate>;
        static  auto New(int width, int height, std::string title)->std::shared_ptr<Win32AppDelegate>;
        virtual void OnInit()   override;
        virtual void OnRender() override;
        virtual void OnUpdate() override {}
        virtual void OnSizeChanged(UINT width, UINT height, bool minimized) override {}
        virtual void OnDestroy()override;
        virtual void OnKeyUp(UINT8 /*key*/)   override {}
        virtual void OnKeyDown(UINT8 /*key*/) override {}
        virtual void OnWindowMoved(int /*x*/, int /*y*/)override {}
        virtual void OnMouseMove(UINT /*x*/, UINT /*y*/) override {}
        virtual void OnLeftButtonDown(UINT /*x*/, UINT /*y*/) override {}
        virtual void OnLeftButtonUp(UINT /*x*/, UINT /*y*/) override {}
        virtual void OnDisplayChanged() override {}
        virtual ~Test1AppDelegate() {}
    protected:
        void InitAssets();
        void FreeAssets();
    private:
        void InitCommands();
        void InitFences();
        void InitMesh();
        void InitAccelerationStructures();
        void InitPipeline();
        void InitStateObject();
        void InitShaderTable();
        void ExecuteCommandList();
        void WaitForGPU();
    private:
        std::shared_ptr<rtlib::DX12Context>       m_Context       = nullptr;
        std::shared_ptr<rtlib::DX12SwapChain>     m_SwapChain     = nullptr;
        rtlib::ComPtr<ID3D12Resource>             m_VB;
        rtlib::ComPtr<ID3D12Resource>             m_IB;
        rtlib::ComPtr<ID3D12PipelineState>        m_PipelineState;
        rtlib::ComPtr<ID3D12RootSignature>        m_RootSignature;
        std::shared_ptr<rtlib::DX12Mesh>          m_Mesh          = nullptr;
        ///commands
        rtlib::ComPtr<ID3D12CommandAllocator>     m_CommandAllocator;
        rtlib::ComPtr<ID3D12GraphicsCommandList5> m_CommandList;
        rtlib::ComPtr<ID3D12Fence>                m_Fence;
        UINT64                                    m_FenceValue = 0;
        rtlib::Wrappers::Event                    m_FenceEvent;

        rtlib::ComPtr<ID3D12Resource>             m_Blas;
        rtlib::ComPtr<ID3D12Resource>             m_Tlas;
        rtlib::ComPtr<ID3D12Resource>             m_RtOutput;
        rtlib::ComPtr<ID3D12DescriptorHeap>       m_SrvUavHeap;
        rtlib::ComPtr<ID3D12StateObject>          m_StateObject;
        rtlib::ComPtr<ID3D12Resource>             m_ShaderTable;
        UINT32                                    m_ShaderTableEntrySize    = 0;
        rtlib::ComPtr<ID3D12RootSignature>        m_GlobalRootSignature;
        UINT                                      m_CbvSrvUavDescriptorSize = 0;
	};
}
#endif