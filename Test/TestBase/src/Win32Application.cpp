#include <Win32Application.h>
#include <Win32AppDelegate.h>

test::Win32Application::Win32Application() noexcept
{
	m_Width  = 0;
	m_Height = 0;
	m_Title = "";
}

auto test::Win32Application::New() -> std::shared_ptr<Win32Application>
{
	return std::make_shared<Win32Application>();
}

auto test::Win32Application::New(int width, int height, std::string title) -> std::shared_ptr<Win32Application>
{
	return std::make_shared<Win32Application>(width, height, title);
}

void test::Win32Application::OnInit()
{
	if (m_Delegate) {
		m_Width  = m_Delegate->GetWidth();
		m_Height = m_Delegate->GetHeight();
		m_Title  = m_Delegate->GetTitle();
	}
	
	WNDCLASSEX wndCls    = {};
	wndCls.cbSize        = sizeof(WNDCLASSEX);
	wndCls.style         = CS_HREDRAW | CS_VREDRAW;
	wndCls.lpfnWndProc   = test::Win32Application::WindowProc;
	wndCls.hInstance     = GetModuleHandle(nullptr);
	wndCls.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wndCls.lpszClassName = "Win32ApplicationClass";
	RegisterClassEx(&wndCls);

	RECT wndRect         = { 0,0,static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, FALSE);
	
	m_Hwnd = CreateWindow(
		wndCls.lpszClassName,
		m_Title.c_str(),
		m_WindowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wndRect.right - wndRect.left,
		wndRect.bottom - wndRect.top,
		nullptr, nullptr,
		wndCls.hInstance,
		this
	);

	if (m_Delegate) {
		m_Delegate->OnInit();
	}
}

void test::Win32Application::OnMainLoop()
{

	ShowWindow(m_Hwnd, SW_SHOW);

	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void test::Win32Application::OnDestroy()
{
	if (m_Delegate) {
		m_Delegate->OnDestroy();
	}
}

void test::Win32Application::OnRun()
{
	try {
		this->OnInit();
		this->OnMainLoop();
		this->OnDestroy();
	}
	catch (std::exception& e) {
		OnDestroy();
	}
}

test::Win32Application::~Win32Application()
{
}

auto test::Win32Application::GetWidth() const -> UINT
{
	return m_Width;
}

auto test::Win32Application::GetHeight()const -> UINT
{
	return m_Height;
}

auto test::Win32Application::GetTitle() const -> LPCSTR
{
	return m_Title.c_str();
}

void test::Win32Application::Delegate(std::shared_ptr<Win32Application> app, std::shared_ptr<Win32AppDelegate> delegate)
{
	if (app && delegate) {
		app->m_Delegate = delegate;
		delegate->m_Application = app;
	}
}

test::Win32Application::Win32Application(int width, int height, std::string title) noexcept
{
	m_Width  = width;
	m_Height = height;
	m_Title  = title;
}

LRESULT test::Win32Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	test::Win32Application* app = reinterpret_cast<test::Win32Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	bool enableDelegate = app && app->m_Delegate;
	switch (uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
	case WM_KEYDOWN:
		if (enableDelegate) {
			app->m_Delegate->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_KEYUP:
		if (enableDelegate) {
			app->m_Delegate->OnKeyUp  (static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_PAINT:
		if (enableDelegate) {
			app->m_Delegate->OnUpdate();
			app->m_Delegate->OnRender();
		}
		return 0;
	case WM_SIZE:
		if (enableDelegate) {
			RECT wndRect = {};
			GetWindowRect(hwnd, &wndRect);
			app->m_Delegate->SetWindowBounds(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
			RECT clientRect = {};
			GetClientRect(hwnd, &clientRect);
			app->m_Delegate->OnSizeChanged(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, wParam == SIZE_MINIMIZED);
		}
		return 0;
	case WM_MOVE:
		if (enableDelegate) {
			RECT wndRect = {};
			GetWindowRect(hwnd, &wndRect);
			app->m_Delegate->SetWindowBounds(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
			int xPos = (int)(short)LOWORD(lParam);
			int yPos = (int)(short)HIWORD(lParam);
			app->m_Delegate->OnWindowMoved(xPos, yPos);
		}
		return 0;
	case WM_DISPLAYCHANGE:
		if (enableDelegate) {
			app->m_Delegate->OnDisplayChanged();
		}
		return 0;
	case WM_MOUSEMOVE:
		if (enableDelegate && static_cast<UINT8>(wParam) == MK_LBUTTON) {
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			app->m_Delegate->OnMouseMove(x, y);
		}
	case WM_LBUTTONDOWN:
		if (enableDelegate) {
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			app->m_Delegate->OnLeftButtonDown(x, y);
		}
		return 0;
	case WM_LBUTTONUP:
		if (enableDelegate) {
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			app->m_Delegate->OnLeftButtonUp(x, y);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
