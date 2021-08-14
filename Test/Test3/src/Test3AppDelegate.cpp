#include "../include/Test3AppDelegate.h"
#include <RTLib/DX12.h>
#include <dxcapi.h>
#include <Test3Config.h>
#include <filesystem>
test::Test3AppDelegate::Test3AppDelegate() :Win32AppDelegate()
{
}

test::Test3AppDelegate::Test3AppDelegate(int width, int height, std::string title) : Win32AppDelegate(width, height, title)
{
	m_Context = std::make_shared<rtlib::DX12Context>(D3D_FEATURE_LEVEL_12_1);
}

auto test::Test3AppDelegate::New() -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Test3AppDelegate>();
}

auto test::Test3AppDelegate::New(int width, int height, std::string title) -> std::shared_ptr<Win32AppDelegate>
{
	return std::make_shared<Test3AppDelegate>(width, height, title);
}

void test::Test3AppDelegate::OnInit()
{
	m_SwapChain = std::make_shared<rtlib::DX12SwapChain>(m_Context.get(), m_Application.lock()->GetWindow(), m_Application.lock()->GetWidth(), m_Application.lock()->GetHeight(), 3);
	m_Context->OnInit();
	m_SwapChain->OnInit();
	InitAssets();
}

void test::Test3AppDelegate::OnRender()
{
#if 0
	m_SwapChain->BeginFrame();
	auto commandList = m_SwapChain->GetCommandList();
	commandList->SetPipelineState(m_PipelineState.Get());
	commandList->SetGraphicsRootSignature(m_RootSignature.Get());
	commandList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, (float)m_SwapChain->GetWidth(), (float)m_SwapChain->GetHeight()));
	commandList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, m_SwapChain->GetWidth(), m_SwapChain->GetHeight()));
	auto rtvh          = m_SwapChain->GetRtvh();
	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
	commandList->OMSetRenderTargets(1, &rtvh, true, nullptr);
	commandList->ClearRenderTargetView(rtvh, clearColor, 0, nullptr);
	m_Mesh->SetDrawCommand(commandList);
	m_SwapChain->EndFrame();
#else
	m_SwapChain->BeginFrame(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);
	rtlib::ComPtr<ID3D12GraphicsCommandList5> commandList5;
	rtlib::ThrowIfFailed(m_SwapChain->GetCommandList()->QueryInterface(IID_PPV_ARGS(&commandList5)));
	{
		ID3D12DescriptorHeap* heaps[] = { m_SrvUavHeap.Get() };
		commandList5->SetDescriptorHeaps(std::size(heaps), std::data(heaps));
	}
	commandList5->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RtOutput.Get(),D3D12_RESOURCE_STATE_COPY_SOURCE,D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	commandList5->SetComputeRootSignature(m_GlobalRootSignature.Get());
	commandList5->SetComputeRootDescriptorTable(0, m_GlobalRootGpuHandle);

	D3D12_DISPATCH_RAYS_DESC rayTraceDesc = {};
	rayTraceDesc.Width = m_SwapChain->GetWidth();
	rayTraceDesc.Height = m_SwapChain->GetHeight();
	rayTraceDesc.Depth = 1;
	rayTraceDesc.RayGenerationShaderRecord.StartAddress = m_RgSbtAddress;
	rayTraceDesc.RayGenerationShaderRecord.SizeInBytes = m_RgSbtSizeInBytes;
	rayTraceDesc.MissShaderTable.StartAddress = m_MsSbtAddress;
	rayTraceDesc.MissShaderTable.SizeInBytes = m_MsSbtSizeInBytes;
	rayTraceDesc.MissShaderTable.StrideInBytes = m_MsSbtStrideInBytes;
	rayTraceDesc.HitGroupTable.StartAddress = m_HgSbtAddress;
	rayTraceDesc.HitGroupTable.SizeInBytes = m_HgSbtSizeInBytes;
	rayTraceDesc.HitGroupTable.StrideInBytes = m_HgSbtStrideInBytes;
	commandList5->SetPipelineState1(m_StateObject.Get());
	commandList5->DispatchRays(&rayTraceDesc);
	
	auto backBuffer = m_SwapChain->GetBackBuffer();
	CD3DX12_RESOURCE_BARRIER barriers[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_RtOutput.Get(),D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_COPY_SOURCE
		)
	};
	commandList5->ResourceBarrier(2, barriers);
	commandList5->CopyResource(backBuffer, m_RtOutput.Get());
	m_SwapChain->EndFrame(D3D12_RESOURCE_STATE_COPY_DEST);
#endif
}

void test::Test3AppDelegate::OnDestroy()
{
	m_SwapChain->WaitForGPU();
	FreeAssets();
	m_SwapChain->OnDestroy();
	m_Context->OnDestroy();
	m_SwapChain.reset();
}

void test::Test3AppDelegate::InitAssets()
{
	InitCommands();
	InitFences();
	InitMesh();
	InitPipeline();
	InitAccelerationStructures();
	InitStateObject();
	InitShaderTable();
}

void test::Test3AppDelegate::FreeAssets()
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

void test::Test3AppDelegate::InitCommands()
{
	rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)
	));
	RTLIB_DX12_NAME(test::Test3AppDelegate::m_CommandAllocator);

	rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_CommandList)
	));
	RTLIB_DX12_NAME(test::Test3AppDelegate::m_CommandList);
}

void test::Test3AppDelegate::InitFences()
{
	rtlib::ThrowIfFailed(
		m_Context->GetDevice()->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))
	);
	RTLIB_DX12_NAME(test::Test3AppDelegate::m_Fence);

	m_FenceValue++;

	m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));

	if (!m_FenceEvent.IsValid()) {
		throw std::runtime_error("Failed To Create Valid Fence Event");
	}
	this->WaitForGPU();
}

void test::Test3AppDelegate::InitMesh()
{
	m_Mesh = rtlib::DX12Mesh::New("Triangle");

	constexpr float vertices[5 * 4] = {
		-1.0f,-1.0f,0.0f,0.0f,0.0f,
		-1.0f, 1.0f,0.0f,0.0f,1.0f,
		 1.0f, 1.0f,0.0f,1.0f,1.0f,
		 1.0f,-1.0f,0.0f,1.0f,0.0f,
	};

	constexpr UINT32 indices[6] = {
		0,1,2,2,3,0
	};

	{
		rtlib::ComPtr<ID3D12Resource> uploadVB;
		rtlib::ComPtr<ID3D12Resource> uploadIB;

		{
			rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_VB))
			);
			rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadVB))
			);
			D3D12_SUBRESOURCE_DATA subResData = {};
			subResData.pData = std::data(vertices);
			subResData.RowPitch = sizeof(vertices);
			subResData.SlicePitch = sizeof(vertices);

			UpdateSubresources<1>(m_CommandList.Get(), m_VB.Get(), uploadVB.Get(), 0, 0, 1, &subResData);
			m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_VB.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
			);
		}
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_VB);

		{
			rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_IB))
			);
			rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadIB))
			);
			D3D12_SUBRESOURCE_DATA subResData = {};
			subResData.pData = std::data(indices);
			subResData.RowPitch = sizeof(indices);
			subResData.SlicePitch = sizeof(indices);

			UpdateSubresources<1>(m_CommandList.Get(), m_IB.Get(), uploadIB.Get(), 0, 0, 1, &subResData);
			m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_IB.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_INDEX_BUFFER)
			);
		}
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_IB);

		this->ExecuteCommandList();
		this->WaitForGPU();

	}

	m_Mesh->SetVertexBufferView(rtlib::DX12VertexBufferView::New(
		m_VB.Get(), DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 5, sizeof(vertices)
	));

	m_Mesh->SetIndexBufferView(
		rtlib::DX12IndexBufferView::New(
			m_IB.Get(), DXGI_FORMAT_R32_UINT, 0, 6, sizeof(indices)
		));
}

void test::Test3AppDelegate::InitAccelerationStructures()
{
	//Acceleration Structure
	if (!m_Context->SupportDXR()) {
		throw std::runtime_error("Failed To Support DXR!");
	}
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	rtlib::ComPtr<ID3D12Device5> device5;
	m_Context->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5));
	auto geometry = m_Mesh->GetRayTracingGeometry();
	geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	rtlib::ComPtr<ID3D12Resource> blasScratchBuffer;
	{
		auto blasInputs = D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS{};
		blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		blasInputs.NumDescs = 1;
		blasInputs.pGeometryDescs = &geometry;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};
		device5->GetRaytracingAccelerationStructurePrebuildInfo(&blasInputs, &blasInfo);
		fmt::print("ResultDataMaxSizeInBytes={}\n", blasInfo.ResultDataMaxSizeInBytes);
		fmt::print("ScratchDataSizeInBytes={}\n", blasInfo.ScratchDataSizeInBytes);
		fmt::print("UpdateScratchDataSizeInBytes={}\n", blasInfo.UpdateScratchDataSizeInBytes);

		rtlib::ThrowIfFailed(device5->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				blasInfo.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			),
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr,
			IID_PPV_ARGS(&m_Blas)
		));

		RTLIB_DX12_NAME(test::Test3AppDelegate::m_Blas);
		rtlib::ThrowIfFailed(device5->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				blasInfo.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&blasScratchBuffer)
		));
		RTLIB_DX12_NAME(blasScratchBuffer);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {};
		blasDesc.Inputs = blasInputs;
		blasDesc.DestAccelerationStructureData = m_Blas->GetGPUVirtualAddress();
		blasDesc.ScratchAccelerationStructureData = blasScratchBuffer->GetGPUVirtualAddress();

		m_CommandList->BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_Blas.Get()));
	}

	rtlib::ComPtr<ID3D12Resource> instanceDescsBuffer;
	rtlib::ComPtr<ID3D12Resource> tlasScratchBuffer;
	{
		auto tlasInputs = D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS{};
		tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		tlasInputs.NumDescs = 1;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasInfo = {};
		device5->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);
		fmt::print("ResultDataMaxSizeInBytes={}\n", tlasInfo.ResultDataMaxSizeInBytes);
		fmt::print("ScratchDataSizeInBytes={}\n", tlasInfo.ScratchDataSizeInBytes);
		fmt::print("UpdateScratchDataSizeInBytes={}\n", tlasInfo.UpdateScratchDataSizeInBytes);

		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.AccelerationStructure = m_Blas->GetGPUVirtualAddress();
		instanceDesc.InstanceID = 0;
		instanceDesc.InstanceContributionToHitGroupIndex = 0;
		instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		DirectX::XMMATRIX m = DirectX::XMMatrixIdentity();
		memcpy(instanceDesc.Transform, &m, sizeof(float) * 3 * 4);
		instanceDesc.InstanceMask = 0xFF;

		rtlib::ThrowIfFailed(device5->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
				D3D12_RESOURCE_FLAG_NONE
			),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&instanceDescsBuffer)
		));
		{
			void* pInstanceDesc;
			instanceDescsBuffer->Map(0, nullptr, &pInstanceDesc);
			std::memcpy(pInstanceDesc, &instanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
			instanceDescsBuffer->Unmap(0, nullptr);
		}


		rtlib::ThrowIfFailed(device5->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				tlasInfo.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			),
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr,
			IID_PPV_ARGS(&m_Tlas)
		));
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_Tlas);
		rtlib::ThrowIfFailed(device5->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				tlasInfo.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&tlasScratchBuffer)
		));
		RTLIB_DX12_NAME(tlasScratchBuffer);

		tlasInputs.InstanceDescs = instanceDescsBuffer->GetGPUVirtualAddress();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {};
		tlasDesc.Inputs = tlasInputs;
		tlasDesc.DestAccelerationStructureData = m_Tlas->GetGPUVirtualAddress();
		tlasDesc.ScratchAccelerationStructureData = tlasScratchBuffer->GetGPUVirtualAddress();

		m_CommandList->BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_Tlas.Get()));

	}
	this->ExecuteCommandList();
	this->WaitForGPU();
}

void test::Test3AppDelegate::InitPipeline()
{
	//shader
	rtlib::ComPtr<IDxcBlob> vs;
	rtlib::ComPtr<IDxcBlob> ps;
	{
		rtlib::ComPtr<IDxcBlobEncoding> shaderBlob;

		rtlib::ComPtr<IDxcLibrary>      library;
		rtlib::ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));

		rtlib::ComPtr<IDxcCompiler>     compiler;
		rtlib::ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));

		UINT32 codePage = 0;
		auto graphicsShaderPath = std::filesystem::path(TEST3_SHADER_PATH)/"Graphics.hlsl";
		rtlib::ThrowIfFailed(
			library->CreateBlobFromFile(
				graphicsShaderPath.wstring().c_str(),&codePage,&shaderBlob
			)
		);
		{
			rtlib::ComPtr<IDxcOperationResult> vsResult;
			auto hr =
				compiler->Compile(
					shaderBlob.Get(),
					graphicsShaderPath.filename().wstring().c_str(),
					L"vsMain",
					L"vs_6_0",
					NULL, 0,
					NULL, 0,
					NULL,
					&vsResult
				);

			RTLIB_IF_SUCCEEDED(hr) {
				vsResult->GetStatus(&hr);
			}
			RTLIB_IF_FAILED(hr) {
				rtlib::ComPtr<IDxcBlobEncoding> errBlob;
				hr = vsResult->GetErrorBuffer(&errBlob);
				RTLIB_IF_SUCCEEDED(hr) {
					if (errBlob) {
						fmt::print("VS Compile Failed With Error: {}\n", (const char*)errBlob->GetBufferPointer());
					}
				}
			}
			rtlib::ThrowIfFailed(vsResult->GetResult(&vs));
		}
		{
			rtlib::ComPtr<IDxcOperationResult> psResult;
			auto hr =
				compiler->Compile(
					shaderBlob.Get(),
					graphicsShaderPath.filename().wstring().c_str(),
					L"psMain",
					L"ps_6_0",
					NULL, 0,
					NULL, 0,
					NULL,
					&psResult
				);

			RTLIB_IF_SUCCEEDED(hr) {
				psResult->GetStatus(&hr);
			}
			RTLIB_IF_FAILED(hr) {
				rtlib::ComPtr<IDxcBlobEncoding> errBlob;
				hr = psResult->GetErrorBuffer(&errBlob);
				if (errBlob) {
					fmt::print("PS Compile Failed With Error: {}\n", (const char*)errBlob->GetBufferPointer());
				}
			}
			rtlib::ThrowIfFailed(psResult->GetResult(&ps));
		}
	}

	//rootSignature
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		rtlib::ComPtr<ID3DBlob> signature;
		rtlib::ComPtr<ID3DBlob> error;
		rtlib::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
	}
	//pipeline
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT      , 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	{
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.InputLayout = { inputElementDescs,std::size(inputElementDescs) };

		psoDesc.VS.pShaderBytecode    = vs->GetBufferPointer();
		psoDesc.VS.BytecodeLength     = vs->GetBufferSize();
		psoDesc.PS.pShaderBytecode    = ps->GetBufferPointer();
		psoDesc.PS.BytecodeLength     = ps->GetBufferSize();

		psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable   = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask            = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets      = 1;
		psoDesc.RTVFormats[0]         = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count      = 1;

		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
	}
}

void test::Test3AppDelegate::InitStateObject()
{
	CD3DX12_STATE_OBJECT_DESC stateObjectDesc = {};
	stateObjectDesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

	rtlib::ComPtr<IDxcBlob>            rs;
	auto dxilLibrary = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	{
		rtlib::ComPtr<IDxcBlobEncoding> shaderBlob;

		rtlib::ComPtr<IDxcLibrary>      library;
		rtlib::ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));

		rtlib::ComPtr<IDxcCompiler>     compiler;
		rtlib::ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));

		UINT32 codePage = 0;
		auto rayTraceShaderPath = std::filesystem::path(TEST3_SHADER_PATH) / "RayTrace3.hlsl";
		rtlib::ThrowIfFailed(
			library->CreateBlobFromFile(rayTraceShaderPath.wstring().c_str(), &codePage, &shaderBlob)
		);
		{
			rtlib::ComPtr<IDxcOperationResult>  rsResult;

			auto hr = compiler->Compile(
				shaderBlob.Get(),
				rayTraceShaderPath.filename().wstring().c_str(),
				L"",
				L"lib_6_3",
				nullptr,
				0,
				nullptr,
				0,
				nullptr,
				&rsResult);


			RTLIB_IF_SUCCEEDED(hr) {
				rsResult->GetStatus(&hr);
			}
			RTLIB_IF_FAILED(hr) {
				rtlib::ComPtr<IDxcBlobEncoding> errBlob;
				hr = rsResult->GetErrorBuffer(&errBlob);
				RTLIB_IF_SUCCEEDED(hr) {
					if (errBlob) {
						fmt::print("RS Compile Failed With Error: {}\n", (const char*)errBlob->GetBufferPointer());
					}
				}
			}
			rtlib::ThrowIfFailed(rsResult->GetResult(&rs));
		}

		dxilLibrary->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rs->GetBufferPointer(), rs->GetBufferSize()));
		dxilLibrary->DefineExport(L"globalRootSignature");
		dxilLibrary->DefineExport(L"rayGen");
		dxilLibrary->DefineExport(L"miss");
		dxilLibrary->DefineExport(L"missRootSignature");
		dxilLibrary->DefineExport(L"missAssociation");
		dxilLibrary->DefineExport(L"chs");
		dxilLibrary->DefineExport(L"hitGroup");
		dxilLibrary->DefineExport(L"hitGroupRootSignature");
		dxilLibrary->DefineExport(L"hitGroupAssociation");
		dxilLibrary->DefineExport(L"shaderConfig");
		dxilLibrary->DefineExport(L"pipelineConfig");
		dxilLibrary->DefineExport(L"stateObjectConfig");
	}
	{
		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateRootSignature(0, rs->GetBufferPointer(), rs->GetBufferSize(), IID_PPV_ARGS(&m_GlobalRootSignature)));
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_GlobalRootSignature);

		const D3D12_STATE_OBJECT_DESC* pStateObjectDesc = &D3D12_STATE_OBJECT_DESC(stateObjectDesc);
		rtlib::ComPtr<ID3D12Device5> device5;
		m_Context->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5));
		rtlib::ThrowIfFailed(device5->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&m_StateObject)));
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_StateObject);
	}
}

void test::Test3AppDelegate::InitShaderTable()
{
	//  Global Resources
	{
		m_CbvSrvUavDescriptorSize = m_Context->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		rtlib::ThrowIfFailed(
			m_Context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(
					DXGI_FORMAT_R8G8B8A8_UNORM,
					m_SwapChain->GetWidth(),
					m_SwapChain->GetHeight(),
					1, 1, 1, 0,
					D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
					D3D12_TEXTURE_LAYOUT_UNKNOWN
				),
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				nullptr,
				IID_PPV_ARGS(&m_RtOutput)
			)
		);
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_RtOutput);

		m_MsBgColor = { 1.0f,1.0f,0.0f };

		auto msConstants = MissConstants{};
		msConstants.bgColor = m_MsBgColor;

		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				sizeof(msConstants)
			),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_MsConstantsBuffer)
		));

		{
			
			UINT8* pConstantsData = nullptr;
			rtlib::ThrowIfFailed(m_MsConstantsBuffer->Map(0, nullptr, (void**)&pConstantsData));
			std::memcpy(pConstantsData, &msConstants, sizeof(msConstants));
			m_MsConstantsBuffer->Unmap(0, nullptr);
		}
	}
	//Descriptor Heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};

		srvUavHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvUavHeapDesc.NodeMask       = 0;
		srvUavHeapDesc.NumDescriptors = 2;
		srvUavHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		rtlib::ThrowIfFailed(
			m_Context->GetDevice()->CreateDescriptorHeap(
				&srvUavHeapDesc, IID_PPV_ARGS(&m_SrvUavHeap)
			)
		);
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_SrvUavHeap);

		auto srvUavCpuHandle  = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_SrvUavHeap->GetCPUDescriptorHandleForHeapStart());
		auto srvUavGpuHandle  = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_SrvUavHeap->GetGPUDescriptorHandleForHeapStart());
		//Global Descriptor Table
		m_GlobalRootGpuHandle = srvUavGpuHandle;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_Context->GetDevice()->CreateUnorderedAccessView(m_RtOutput.Get(), nullptr, &uavDesc, srvUavCpuHandle);
		srvUavCpuHandle.Offset(1, m_CbvSrvUavDescriptorSize);
		srvUavGpuHandle.Offset(1, m_CbvSrvUavDescriptorSize);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = m_Tlas->GetGPUVirtualAddress();
		m_Context->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, srvUavCpuHandle);
	}
	rtlib::ComPtr<ID3D12StateObjectProperties> stateObjectProp;
	//Shader Binding Table
	m_StateObject.As(&stateObjectProp);
	{
		m_RgSbtSizeInBytes = rtlib::getDX12AlignedSize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_RgSbtSizeInBytes),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_RgSbtBuffer))
		);
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_RgSbtBuffer);

		m_RgSbtAddress    = m_RgSbtBuffer->GetGPUVirtualAddress();
		UINT8* pRgSbtData = nullptr;
		rtlib::ThrowIfFailed(m_RgSbtBuffer->Map(0, nullptr, (void**)&pRgSbtData));
		std::memcpy(pRgSbtData, stateObjectProp->GetShaderIdentifier(L"rayGen"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		m_RgSbtBuffer->Unmap(0, nullptr);
	}
	{
		m_MsSbtStrideInBytes = rtlib::getDX12AlignedSize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(D3D12_GPU_VIRTUAL_ADDRESS), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		m_MsSbtSizeInBytes   = m_MsSbtStrideInBytes * 1;
		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_MsSbtSizeInBytes),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_MsSbtBuffer))
		);
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_MsSbtBuffer);

		m_MsSbtAddress    = m_MsSbtBuffer->GetGPUVirtualAddress();
		UINT8* pMsSbtData = nullptr;
		rtlib::ThrowIfFailed(m_MsSbtBuffer->Map(0, nullptr, (void**)&pMsSbtData));
		std::memcpy(pMsSbtData, stateObjectProp->GetShaderIdentifier(L"miss"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		{
			auto msConstantsBufferAddress = m_MsConstantsBuffer->GetGPUVirtualAddress();
			std::memcpy(pMsSbtData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, &msConstantsBufferAddress, sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
		}
		
		m_MsSbtBuffer->Unmap(0, nullptr);
	}
	{
		m_HgSbtStrideInBytes = rtlib::getDX12AlignedSize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(D3D12_GPU_VIRTUAL_ADDRESS)*2, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		m_HgSbtSizeInBytes   = m_HgSbtStrideInBytes * 1 * 1;
		rtlib::ThrowIfFailed(m_Context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_HgSbtSizeInBytes),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_HgSbtBuffer))
		);
		RTLIB_DX12_NAME(test::Test3AppDelegate::m_HgSbtBuffer);

		m_HgSbtAddress = m_HgSbtBuffer->GetGPUVirtualAddress();
		auto vertAddress = m_Mesh->GetVertexBufferView()->GetResource()->GetGPUVirtualAddress();
		auto indxAddress = m_Mesh->GetIndexBufferView()->GetResource()->GetGPUVirtualAddress();
		UINT8* pHgSbtData = nullptr;
		rtlib::ThrowIfFailed(m_HgSbtBuffer->Map(0, nullptr, (void**)&pHgSbtData));
		std::memcpy(pHgSbtData, stateObjectProp->GetShaderIdentifier(L"hitGroup"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		std::memcpy(pHgSbtData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,&vertAddress, sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
		std::memcpy(pHgSbtData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(D3D12_GPU_VIRTUAL_ADDRESS), &indxAddress, sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
		m_HgSbtBuffer->Unmap(0, nullptr);
	}

}

void test::Test3AppDelegate::ExecuteCommandList() {
	rtlib::ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* commandLists[] = { m_CommandList.Get() };
	m_Context->GetGCmdQueue()->ExecuteCommandLists(1, commandLists);
}

void test::Test3AppDelegate::WaitForGPU() {
	
	if (m_FenceEvent.IsValid()) {
		const UINT curFenceValue = m_FenceValue;
		RTLIB_IF_SUCCEEDED(m_Context->GetGCmdQueue()->Signal(m_Fence.Get(), curFenceValue));
		if(m_Fence->GetCompletedValue() < curFenceValue){
			m_Fence->SetEventOnCompletion(curFenceValue, m_FenceEvent.Get());
			WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
		}
		m_FenceValue++;
	}
}