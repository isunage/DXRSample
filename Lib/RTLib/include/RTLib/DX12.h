#ifndef RTLIB_DX12_H
#define RTLIB_DX12_H
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>
#include <fmt/format.h>
#include <stdexcept>
#include <system_error>
#define RTLIB_IF_SUCCEEDED(HRES) if(SUCCEEDED(HRES))
#define RTLIB_IF_FAILED(HRES) if(FAILED(HRES))
#define RTLIB_DX12_NAME(VAR) rtlib::SetNameToDX12Obj(VAR,L#VAR)
#define RTLIB_DX12_NAME_INDEXED(VAR,IND) rtlib::SetNameToDX12Obj(VAR[IND],L#VAR,IND)
namespace rtlib{
    using namespace Microsoft::WRL;
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
}
#endif