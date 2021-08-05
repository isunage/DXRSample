#include "../include/Test1AppDelegate.h"

test::Test1AppDelegate::Test1AppDelegate() :Win32AppDelegate()
{
}

test::Test1AppDelegate::Test1AppDelegate(int width, int height, std::string title) : Win32AppDelegate(width, height, title)
{
	m_Context = std::make_shared<rtlib::DX12Context>(D3D_FEATURE_LEVEL_12_1);
}

auto test::Test1AppDelegate::New() -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Test1AppDelegate>();
}

auto test::Test1AppDelegate::New(int width, int height, std::string title) -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Test1AppDelegate>(width, height, title);
}

void test::Test1AppDelegate::OnInit()
{
	m_SwapChain = std::make_shared<rtlib::DX12SwapChain>(m_Context.get(), m_Application.lock()->GetWindow(), m_Application.lock()->GetWidth(), m_Application.lock()->GetHeight(), 3);
	m_Context->OnInit();
	m_SwapChain->OnInit();
}

void test::Test1AppDelegate::OnRender()
{
	m_SwapChain->BeginFrame();
	auto commandList = m_SwapChain->GetCommandList();
	auto rtvh = m_SwapChain->GetRtvh();
	float clearColor[] = { 1.0f,0.0f,1.0f,1.0f };
	commandList->OMSetRenderTargets(1, &rtvh, true, nullptr);
	commandList->ClearRenderTargetView(rtvh, clearColor, 0, nullptr);
	m_SwapChain->EndFrame();
}

void test::Test1AppDelegate::OnDestroy()
{
	m_SwapChain->OnDestroy();
	m_Context->OnDestroy();
	m_SwapChain.reset();
}
