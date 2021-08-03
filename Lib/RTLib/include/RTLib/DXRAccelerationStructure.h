#ifndef RTLIB_DXR_ACCELERATION_STRUCTURE_H
#define RTLIB_DXR_ACCELERATION_STRUCTURE_H
#include "DXR.h"
namespace rtlib{
    class DXRAccelerationStructure {
    public:
        virtual void OnInit()    = 0;
        virtual void OnBuild()   = 0;
        virtual void OnUpdate()  = 0;
        virtual void OnDestroy() = 0;
        virtual ~DXRAccelerationStructure(){}
    protected:
        ComPtr<ID3D12Resource> m_Scratch;
        ComPtr<ID3D12Resource> m_AccelerationSturcture;
        ComPtr<ID3D12Resource> m_Update;
        UINT64                 m_ResultDataMaxSizeInBytes;
        UINT64                 m_UpdateDataMaxSizeInBytes;
    };
    class DXRTopLevelAccelerationStructure   :public DXRAccelerationStructure {
    public:
        virtual void OnInit()   override;
        virtual void OnBuild()  override;
        virtual void OnUpdate() override;
        virtual void OnDestroy()override;
        virtual ~DXRTopLevelAccelerationStructure(){}
    protected:
    };
    class DXRBottomLevelAccelerationStructure:public DXRAccelerationStructure {
    public:
        virtual void OnInit()   override;
        virtual void OnBuild()  override;
        virtual void OnUpdate() override;
        virtual void OnDestroy()override;
        virtual ~DXRBottomLevelAccelerationStructure(){}
    protected:
    };
}
#endif