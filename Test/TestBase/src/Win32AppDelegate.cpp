#include <Win32AppDelegate.h>
#include <Win32Application.h>

test::Win32AppDelegate::~Win32AppDelegate()
{
}

auto test::Win32AppDelegate::GetWidth() const -> UINT
{
	return m_Width;
}

auto test::Win32AppDelegate::GetHeight() const -> UINT
{
	return m_Height;
}

auto test::Win32AppDelegate::GetTitle() const -> LPCSTR
{
	return m_Title.c_str();
}

void test::Win32AppDelegate::SetWindowBounds(int left, int top, int right, int bottom)
{
	m_WindowBounds.left   = static_cast<LONG>(left);
	m_WindowBounds.right  = static_cast<LONG>(right);
	m_WindowBounds.top    = static_cast<LONG>(top);
	m_WindowBounds.bottom = static_cast<LONG>(bottom);
}

test::Win32AppDelegate::Win32AppDelegate(int width, int height, std::string title) noexcept
{
	m_Width  = width;
	m_Height = height;
	m_Title  = title;
}

test::Win32AppDelegate::Win32AppDelegate() noexcept
{
	m_Width = 0;
	m_Height = 0;
	m_Title  = "";
}

auto test::Win32AppDelegate::New() -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Win32AppDelegate>();
}

auto test::Win32AppDelegate::New(int width, int height, std::string title) -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Win32AppDelegate>(width,height,title);
}
