#ifndef TEST_BASE_WIN32_APP_DELEGATE_H
#define TEST_BASE_WIN32_APP_DELEGATE_H
#include "Win32Application.h"
#include <memory>
namespace test {
    class Win32Application;
    class Win32AppDelegate {
    private:
        friend class Win32Application;
    public:
        Win32AppDelegate(int width, int height, std::string title)noexcept;
        Win32AppDelegate() noexcept;
        static auto New() -> std::shared_ptr<Win32AppDelegate>;
        static auto New(int width, int height, std::string title) ->std::shared_ptr<Win32AppDelegate>;
        virtual void OnInit()   {}
        virtual void OnRender() {}
        virtual void OnUpdate() {}
        virtual void OnSizeChanged(UINT width, UINT height, bool minimized) {}
        virtual void OnDestroy(){}
        virtual void OnKeyUp(UINT8 /*key*/){}
        virtual void OnKeyDown(UINT8 /*key*/){}
        virtual void OnWindowMoved(int /*x*/, int /*y*/) {}
        virtual void OnMouseMove(UINT /*x*/, UINT /*y*/) {}
        virtual void OnLeftButtonDown(UINT /*x*/, UINT /*y*/) {}
        virtual void OnLeftButtonUp(UINT /*x*/, UINT /*y*/) {}
        virtual void OnDisplayChanged() {}
        virtual ~Win32AppDelegate();
        auto GetWidth ()const->UINT;
        auto GetHeight()const->UINT;
        auto GetTitle ()const->LPCSTR;
        auto GetWindowBounds()const->RECT { return m_WindowBounds;  }
        void SetWindowBounds(int left, int top, int right, int bottom);

    protected:
        std::weak_ptr<Win32Application> m_Application;
        UINT                            m_Width;
        UINT                            m_Height;
        std::string                     m_Title;
        RECT                            m_WindowBounds;

    };
}
#endif