#include "../include/RTLib/Utils/ObjModelLoader.h"
#include <iostream>

bool rtlib::utils::ObjModelLoader::OnLoad(const char* objFilePath, const char* mtlFileDir)
{
	tinyobj::ObjReaderConfig readerConfig = {};
	readerConfig.triangulate              = true;
	readerConfig.triangulation_method     ="simple";
	readerConfig.vertex_color             = false;
	readerConfig.mtl_search_path          = mtlFileDir;
	tinyobj::ObjReader       reader       = {};
	if (!reader.ParseFromFile(objFilePath, readerConfig))
	{
		return false;
	}
	if (!reader.Valid()) {
		return false;
	}
	if (!reader.Warning().empty()) {
		std::cerr << "Warning: " << reader.Warning() << std::endl;
	}
	auto attrib    = reader.GetAttrib();
	auto materials = reader.GetMaterials();
	auto shapes    = reader.GetShapes();
	
	return true;
}

void rtlib::utils::ObjModelLoader::OnDestroy()
{
	m_SoAVertices.clear();
	m_SoAIndices .clear();
	m_AoSVertices =    {};
	m_AoSIndices .clear();
}
