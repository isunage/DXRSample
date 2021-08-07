#ifndef RTLIB_DXR_ACCELERATION_STRUCTURE_H
#define RTLIB_DXR_ACCELERATION_STRUCTURE_H
#include "DXR.h"
#include <string>
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
        virtual void OnInit()   override {}
        virtual void OnBuild()  override {}
        virtual void OnUpdate() override {}
        virtual void OnDestroy()override {}
        virtual ~DXRTopLevelAccelerationStructure(){}
    protected:

    };
    class DXRBottomLevelAccelerationStructure:public DXRAccelerationStructure {
    private:
        using MeshMap = std::unordered_map<std::string, std::shared_ptr<DX12Mesh>>;
    public:
        virtual void OnInit()   override {}
        virtual void OnBuild()  override {}
        virtual void OnUpdate() override {}
        virtual void OnDestroy()override {}
        virtual ~DXRBottomLevelAccelerationStructure(){}

        void SetMesh(const std::shared_ptr<DX12Mesh>& mesh) {
            if (m_MeshMap.count(mesh->GetName()) == 0) {
                m_MeshMap[mesh->GetName()] = mesh;
            }
        }
        auto GetMesh(const std::string& name)const -> const std::shared_ptr<DX12Mesh>&
        {
            if (m_MeshMap.count(name) > 0) {
                return m_MeshMap.at(name);
            }
            else {
                throw std::runtime_error("Failed To Get Valid Mesh!");
            }
        }
    protected:
        MeshMap m_MeshMap =  {};
    };
}
#endif