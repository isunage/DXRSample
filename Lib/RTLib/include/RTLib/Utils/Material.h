#ifndef RTLIB_UTILS_MATERIAL_H
#define RTLIB_UTILS_MATERIAL_H
#include <DirectXMath.h>
#include <unordered_map>
#include <array>
namespace rtlib
{
	namespace utils
	{
		class Material
		{
		public:
			using InternalUInt32 = uint32_t;
			using InternalString = std::string;
			using InternalFloat1 = float;
			using InternalFloat2 = std::array<float, 2>;
			using InternalFloat3 = std::array<float, 3>;
			using InternalFloat4 = std::array<float, 4>;
		public:
			//UINT32
			void SetUInt32(const char* key,const  InternalUInt32& val)
			{
				m_UInt32Map[key] = val;
			}
			auto GetUInt32(const char* key)const->InternalUInt32
			{
				return m_UInt32Map.at(key);
			}
			//STRING
			void SetString(const char* key,const  InternalString& val)
			{
				m_StringMap[key] = val;
			}
			auto GetString(const char* key)const->InternalString
			{
				return m_StringMap.at(key);
			}
			//Float1
			void SetFloat1(const char* key,const  InternalFloat1& val)
			{
				m_Float1Map[key] = val;
			}
			auto GetFloat1(const char* key)const->InternalFloat1
			{
				return m_Float1Map.at(key);
			}
			//Float2
			void SetFloat2(const char* key,const  InternalFloat2& val)
			{
				m_Float2Map[key] = val;
			}
			auto GetFloat2(const char* key)const->InternalFloat2
			{
				return m_Float2Map.at(key);
			}
			template<typename TypeFloat2>
			void SetFloat2As(const char* key,const TypeFloat2& val) {
				InternalFloat2 val2 = SetFloat2(key);
				std::memcpy(&val2, &val, sizeof(InternalFloat2));
				SetFloat2(key, val2);
			}
			template<typename TypeFloat2>
			auto GetFloat2As(const char* key)const -> TypeFloat2 {
				TypeFloat2     val  = {};
				InternalFloat2 val2 = GetFloat2(key);
				std::memcpy(&val,&val2, sizeof(InternalFloat2));
				return val;
			}
			//Float3
			void SetFloat3(const char* key,const  InternalFloat3& val)
			{
				m_Float3Map[key] = val;
			}
			auto GetFloat3(const char* key)const->InternalFloat3
			{
				return m_Float3Map.at(key);
			}
			template<typename TypeFloat3>
			void SetFloat3As(const char* key, const TypeFloat3& val) {
				InternalFloat3 val3 = SetFloat3(key);
				std::memcpy(&val3, &val, sizeof(InternalFloat3));
				SetFloat3(key, val3);
			}
			template<typename TypeFloat3>
			auto GetFloat3As(const char* key)const -> TypeFloat3 {
				TypeFloat3     val  = {};
				InternalFloat2 val3 = GetFloat3(key);
				std::memcpy(&val, &val3, sizeof(InternalFloat3));
				return val;
			}
			//Float4
			void SetFloat4(const char* key,const  InternalFloat4& val)
			{
				m_Float4Map[key] = val;
			}
			auto GetFloat4(const char* key)const->InternalFloat4
			{
				return m_Float4Map.at(key);
			}
			template<typename TypeFloat4>
			void SetFloat4As(const char* key, const TypeFloat4& val) {
				InternalFloat4 val4 = SetFloat4(key);
				std::memcpy(&val4, &val, sizeof(InternalFloat4));
				SetFloat3(key, val4);
			}
			template<typename TypeFloat4>
			auto GetFloat4As(const char* key)const -> TypeFloat4 {
				TypeFloat4     val = {};
				InternalFloat4 val4= GetFloat4(key);
				std::memcpy(&val, &val4, sizeof(InternalFloat4));
				return val;
			}
		private:
			std::unordered_map<std::string, InternalUInt32> m_UInt32Map = {};
			std::unordered_map<std::string, InternalString> m_StringMap = {};
			std::unordered_map<std::string, InternalFloat1> m_Float1Map = {};
			std::unordered_map<std::string, InternalFloat2> m_Float2Map = {};
			std::unordered_map<std::string, InternalFloat3> m_Float3Map = {};
			std::unordered_map<std::string, InternalFloat4> m_Float4Map = {};
		};
	}
}
#endif