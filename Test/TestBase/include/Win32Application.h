#ifndef TEST_BASE_WIN32_APPLICATION_H
#define TEST_BASE_WIN32_APPLICATION_H
#include <Windows.h>
#include <memory>
#include <string>
namespace test {
    class Win32AppDelegate;
    class Win32Application {
    private:
        friend class Win32AppDelegate;
    public:
        Win32Application(int width, int height, std::string title)noexcept;
        Win32Application() noexcept;
        static  auto New() -> std::shared_ptr<Win32Application>;
        static  auto New(int width, int height, std::string title)->std::shared_ptr<Win32Application>;
        virtual void OnInit();
        virtual void OnMainLoop();
        virtual void OnDestroy();
        void         OnRun();
        virtual ~Win32Application();
        auto    GetWidth() const->UINT;
        auto    GetHeight()const->UINT;
        auto    GetTitle() const->LPCSTR;
        static void Delegate(std::shared_ptr<Win32Application> app, std::shared_ptr<Win32AppDelegate> delegate);
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    protected:
        UINT                              m_Width;
        UINT                              m_Height;
        std::string                       m_Title;
        std::shared_ptr<Win32AppDelegate> m_Delegate;
        HWND                              m_Hwnd;
        bool                              m_FullScreenMode;
        UINT                              m_WindowStyle = WS_OVERLAPPEDWINDOW;
        RECT                              m_WindowBounds;
    };
}
#endif