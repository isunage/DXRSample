#include "../include/Test4AppDelegate.h"
#include <RTLib/DX12.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <Test4Config.h>
#include <filesystem>

test::Test4AppDelegate::Test4AppDelegate(): Win32AppDelegate()
{
}

test::Test4AppDelegate::Test4AppDelegate(int width, int height, std::string title):Win32AppDelegate(width,height,title)
{
	m_Context = std::make_shared<rtlib::DX12Context>(D3D_FEATURE_LEVEL_12_1);
}

auto test::Test4AppDelegate::New() -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<test::Test4AppDelegate>();
}

auto test::Test4AppDelegate::New(int width, int height, std::string title) -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<test::Test4AppDelegate>(width, height, title);
}

void test::Test4AppDelegate::OnInit()
{
	m_SwapChain = std::make_shared<rtlib::DX12SwapChain>(
		m_Context.get(),
		m_Application.lock()->GetWindow(),
		m_Application.lock()->GetWidth() ,
		m_Application.lock()->GetHeight(),3
	);
	m_Context->OnInit();
	m_SwapChain->OnInit();
	this->InitAssets();
}

void test::Test4AppDelegate::OnRender()
{

}

void test::Test4AppDelegate::OnDestroy()
{
	m_SwapChain->WaitForGPU();
	this->FreeAssets();
	m_SwapChain->OnDestroy();
	m_Context->OnDestroy();
	m_SwapChain.reset();
	m_Context.reset();
}

void test::Test4AppDelegate::InitAssets()
{
	this->InitCommands();
	this->InitFences();
	this->InitMesh();
	this->InitPipeline();
	this->InitAccelerationStructures();
	this->InitStateObject();
	this->InitShaderTable();
}

void test::Test4AppDelegate::FreeAssets()
{
	m_Mesh.reset();
	m_VB.Reset();
	m_IB.Reset();
	m_RootSignature.Reset();
	m_PipelineState.Reset();
	m_CommandAllocator.Reset();
	m_CommandList.Reset();
	m_Fence.Reset();
}

void test::Test4AppDelegate::InitCommands()
{

}

void test::Test4AppDelegate::InitFences()
{
	rtlib::ThrowIfFailed(
		m_Context->GetDevice()->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))
	);
	RTLIB_DX12_NAME(test::Test4AppDelegate::m_Fence);

	m_FenceValue++;
	m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));

	if (!m_FenceEvent.IsValid()) {
		throw std::runtime_error("Failed To Create Valid Fence Event");
	}
	this->WaitForGPU();
}

void test::Test4AppDelegate::InitMeshes()
{
}

void test::Test4AppDelegate::InitPipeline()
{
}

void test::Test4AppDelegate::InitAccelerationStructures()
{
}

void test::Test4AppDelegate::InitStateObject()
{
}

void test::Test4AppDelegate::InitShaderTable()
{
}

void test::Test4AppDelegate::ExecuteCommandList()
{
}

void test::Test4AppDelegate::WaitForGPU()
{
	if (m_FenceEvent.IsValid()) {
		const UINT curFenceValue = m_FenceValue;
		RTLIB_IF_SUCCEEDED(m_Context->GetGCmdQueue()->Signal(m_Fence.Get(), curFenceValue));
		if (m_Fence->GetCompletedValue() < curFenceValue) {
			m_Fence->SetEventOnCompletion(curFenceValue, m_FenceEvent.Get());
			WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
		}
		m_FenceValue++;
	}
}
