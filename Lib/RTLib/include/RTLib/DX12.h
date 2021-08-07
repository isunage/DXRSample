#ifndef RTLIB_DX12_H
#define RTLIB_DX12_H
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <fmt/format.h>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#define RTLIB_IF_SUCCEEDED(HRES) if(SUCCEEDED(HRES))
#define RTLIB_IF_FAILED(HRES) if(FAILED(HRES))
#define RTLIB_DX12_NAME(VAR) rtlib::SetNameToDX12Obj(VAR,L#VAR)
#define RTLIB_DX12_NAME_INDEXED(VAR,IND) rtlib::SetNameToDX12Obj(VAR[IND],L#VAR,IND)
namespace rtlib{
    using namespace Microsoft::WRL;
    using namespace DirectX;
    inline constexpr UINT getDX12AlignedSize(UINT sizeInBytes, UINT alignment) {
        return ((sizeInBytes + alignment - 1) / alignment) * alignment;
    }
    class HrException:public std::runtime_error{
    public:
        HrException(HRESULT ec)noexcept :std::runtime_error(std::system_category().message(ec)), m_ErrorCode{ec}{}
        auto GetErrorCode()const -> HRESULT { return m_ErrorCode; }
    private:
        HRESULT m_ErrorCode;
    };
    inline void ThrowIfFailed(HRESULT result) {
        if (result != S_OK) {
            throw HrException(result);
        }
    }
    inline void ThrowIfFailed(HRESULT result, const std::string& desc) {
        if (result != S_OK) {
#ifdef _DEBUG
            fmt::print(desc);
#endif
            throw HrException(result);
        }
    }
#ifdef _DEBUG
    inline void SetNameToDX12Obj(ComPtr<ID3D12Object> obj, LPWSTR str) {
        if (obj) {
            obj->SetName(str);
        }
    }
    inline void SetNameToDX12Obj(ComPtr<ID3D12Object> obj, LPWSTR baseStr, UINT index) {
        if (obj) {
            std::wstring str = std::wstring(baseStr) + L"[" + std::to_wstring(index) + L"]";
            obj->SetName(str.data());
        }
    }
    inline void SetNameToDX12Obj(ID3D12Object* obj, LPWSTR str) {
        if (obj) {
            obj->SetName(str);
        }
    }
    inline void SetNameToDX12Obj(ID3D12Object* obj, LPWSTR baseStr, UINT index) {
        if (obj) {
            std::wstring str = std::wstring(baseStr) + L"[" + std::to_wstring(index) + L"]";
            obj->SetName(str.data());
        }
    }
#else
    inline void SetNameToDX12Obj(ComPtr<ID3D12Object> obj, LPWSTR str) {
    }
    inline void SetNameToDX12Obj(ComPtr<ID3D12Object> obj, LPWSTR baseStr, UINT index) {
}
    inline void SetNameToDX12Obj(ID3D12Object* obj, LPWSTR str) {
    }
    inline void SetNameToDX12Obj(ID3D12Object* obj, LPWSTR baseStr, UINT index) {
    }
#endif

    class DX12VertexBufferView {
    public:
        DX12VertexBufferView(ID3D12Resource* resource,
            DXGI_FORMAT            format,
            UINT                   offsetInBytes,
            UINT                   strideInBytes,
            UINT                   sizeInBytes)noexcept :m_Resource{ resource },
            m_Format{ format },
            m_OffsetInBytes{ offsetInBytes },
            m_StrideInBytes{ strideInBytes },
            m_SizeInBytes{ sizeInBytes }{
            m_NumElements = m_SizeInBytes / m_StrideInBytes;
        }
        static auto New(ID3D12Resource* resource,
            DXGI_FORMAT            format,
            UINT                   offsetInBytes,
            UINT                   strideInBytes,
            UINT                   sizeInBytes) -> std::shared_ptr<DX12VertexBufferView> {
            if (resource) {
                auto desc = resource->GetDesc();
                auto requiredSizeInBytes = offsetInBytes + sizeInBytes;
                if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
                    && requiredSizeInBytes <= desc.Width
                    && IsValidFormat(format))
                {
                    return std::make_shared<DX12VertexBufferView>(resource, format, offsetInBytes, strideInBytes, sizeInBytes);
                }

            }
            return nullptr;
        }
        auto GetResource()const -> ID3D12Resource* { return m_Resource; }
        auto GetVirtualAddreess()const -> D3D12_GPU_VIRTUAL_ADDRESS {
            return m_Resource->GetGPUVirtualAddress() + m_OffsetInBytes;
        }
        auto GetStrideInBytes()const -> UINT { return m_StrideInBytes; }
        auto GetOffsetInBytes()const -> UINT { return m_OffsetInBytes; }
        auto GetSizeInBytes()const -> UINT { return m_SizeInBytes; }
        auto GetNumElements()const -> UINT { return m_NumElements; }
        auto GetFormat()const -> DXGI_FORMAT { return m_Format; }
        auto GetView()const -> D3D12_VERTEX_BUFFER_VIEW {
            D3D12_VERTEX_BUFFER_VIEW view;
            view.BufferLocation = GetVirtualAddreess();
            view.SizeInBytes = m_SizeInBytes;
            view.StrideInBytes = m_StrideInBytes;
            return view;
        }
    private:
        static bool IsValidFormat(DXGI_FORMAT format) {
            if (format == DXGI_FORMAT_R32G32_FLOAT) return true;
            if (format == DXGI_FORMAT_R32G32B32_FLOAT) return true;
            if (format == DXGI_FORMAT_R16G16_FLOAT)  return true;
            if (format == DXGI_FORMAT_R16G16_SNORM) return true;
            if (format == DXGI_FORMAT_R16G16B16A16_SNORM) return true;
            return false;
        }
    private:
        DXGI_FORMAT            m_Format = DXGI_FORMAT_UNKNOWN;
        UINT                   m_OffsetInBytes = 0;
        UINT                   m_StrideInBytes = 0;
        UINT                   m_SizeInBytes = 0;
        UINT                   m_NumElements = 0;
        ID3D12Resource* m_Resource = nullptr;
    };
    class DX12IndexBufferView {
    public:
        DX12IndexBufferView(ID3D12Resource* resource,
            DXGI_FORMAT     format,
            UINT            offsetInBytes,
            UINT            numElements,
            UINT            sizeInBytes)noexcept :m_Resource{ resource },
            m_Format{ format },
            m_OffsetInBytes{ offsetInBytes },
            m_NumElements{ numElements },
            m_SizeInBytes{ sizeInBytes }{
        }
        static auto New(ID3D12Resource* resource,
            DXGI_FORMAT            format,
            UINT                   offsetInBytes,
            UINT                   numElements,
            UINT                   sizeInBytes) -> std::shared_ptr<DX12IndexBufferView> {
            if (resource) {
                auto desc = resource->GetDesc();
                auto requiredSizeInBytes = offsetInBytes + sizeInBytes;

                if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
                    && requiredSizeInBytes <= desc.Width
                    && IsValidFormat(format))
                {

                    return std::make_shared<DX12IndexBufferView>(resource, format, offsetInBytes, numElements, sizeInBytes);
                }
            }
            return nullptr;
        }
        auto GetResource()const -> ID3D12Resource* { return m_Resource; }
        auto GetVirtualAddreess()const -> D3D12_GPU_VIRTUAL_ADDRESS {
            return m_Resource->GetGPUVirtualAddress() + m_OffsetInBytes;
        }
        auto GetOffsetInBytes()const -> UINT { return m_OffsetInBytes; }
        auto GetSizeInBytes()const -> UINT { return m_SizeInBytes; }
        auto GetNumElements()const -> UINT { return m_NumElements; }
        auto GetFormat()const -> DXGI_FORMAT { return m_Format; }
        auto GetView()const -> D3D12_INDEX_BUFFER_VIEW {
            D3D12_INDEX_BUFFER_VIEW view;
            view.BufferLocation = GetVirtualAddreess();
            view.Format = m_Format;
            view.SizeInBytes = m_SizeInBytes;
            return view;
        }
    private:
        static bool IsValidFormat(DXGI_FORMAT format) {
            if (format == DXGI_FORMAT_R16_UINT) return true;
            if (format == DXGI_FORMAT_R32_UINT) return true;
            return false;
        }
    private:
        DXGI_FORMAT     m_Format = DXGI_FORMAT_UNKNOWN;
        UINT            m_OffsetInBytes = 0;
        UINT            m_SizeInBytes = 0;
        UINT            m_NumElements = 0;
        ID3D12Resource* m_Resource = nullptr;
    };
    class DX12Mesh {
    private:
        using VertexBufferViewPtr = std::shared_ptr<DX12VertexBufferView>;
        using  IndexBufferViewPtr = std::shared_ptr<DX12IndexBufferView>;
    public:
        DX12Mesh(const std::string& name="") :m_Name{name} {}
        static auto New(const std::string& name = "")->std::shared_ptr<DX12Mesh> {
            return std::make_shared<DX12Mesh>(name);
        }
        auto GetName()const->std::string {
            return m_Name;
        }
        void SetName(const std::string& name = "") {
            m_Name = name;
        }
        void SetVertexBufferView(const VertexBufferViewPtr& view) {
            m_VbView = view;
        }
        auto GetVertexBufferView()const ->const VertexBufferViewPtr& {
            return m_VbView;
        }
        void SetIndexBufferView(const IndexBufferViewPtr& view) {
            m_IbView = view;
        }
        auto GetIndexBufferView()const ->const IndexBufferViewPtr& {
            return m_IbView;
        }
        auto GetModelMatrix()const -> XMFLOAT3X4 {
            return m_Models;
        }
        void SetModelMatrix(const XMFLOAT3X4& models) {
            m_Models = models;
        }
        void SetDrawCommand(ID3D12GraphicsCommandList* commandList)const {
            auto vbView = m_VbView->GetView();
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->IASetVertexBuffers(0, 1, &vbView);
            if (m_IbView) {
                auto ibView = m_IbView->GetView();
                commandList->IASetIndexBuffer(&ibView);
                commandList->DrawIndexedInstanced(m_IbView->GetNumElements(), 1, 0, 0, 0);
            }
            else {
                commandList->DrawInstanced(m_IbView->GetNumElements(), 1, 0, 0);
            }

        }
        auto GetRayTracingGeometry()const -> D3D12_RAYTRACING_GEOMETRY_DESC {
            D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
            geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometry.Triangles.VertexFormat = m_VbView->GetFormat();
            geometry.Triangles.VertexBuffer.StartAddress = m_VbView->GetVirtualAddreess();
            geometry.Triangles.VertexBuffer.StrideInBytes = m_VbView->GetStrideInBytes();
            geometry.Triangles.VertexCount = m_VbView->GetNumElements();
            if (m_IbView) {
                geometry.Triangles.IndexBuffer = m_IbView->GetVirtualAddreess();
                geometry.Triangles.IndexCount = m_IbView->GetNumElements();
                geometry.Triangles.IndexFormat = m_IbView->GetFormat();
            }
            geometry.Triangles.Transform3x4 = 0;
            return geometry;
        }
    private:
        std::string         m_Name   = {};
        VertexBufferViewPtr m_VbView = {};
        IndexBufferViewPtr  m_IbView = {};
        XMFLOAT3X4          m_Models = {};
    };

}
#endif