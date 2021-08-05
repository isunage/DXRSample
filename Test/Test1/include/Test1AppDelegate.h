#ifndef TEST1_APP_DELEGATE_H
#define TEST1_APP_DELEGATE_H
#include <Win32AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
#include <dxcapi.h>
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
    private:
        std::shared_ptr<rtlib::DX12Context>   m_Context   = nullptr;
        std::shared_ptr<rtlib::DX12SwapChain> m_SwapChain = nullptr;
        Microsoft::WRL::ComPtr<IDxcLibrary>   m_Library;
	};
}
#endif