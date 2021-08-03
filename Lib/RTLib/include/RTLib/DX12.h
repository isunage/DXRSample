#ifndef RTLIB_DX12_H
#define RTLIB_DX12_H
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>
#include <stdexcept>
#include <system_error>
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
}
#endif