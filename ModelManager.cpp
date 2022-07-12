#include "XUtil.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void Model::CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename)
{

}

void Model::CreateFromGeometry(Model& model, ID3D11Device* device, const GeometryData& data, bool isDynamic)
{
}

void Model::SetDebugObjectName(std::string_view name)
{
}

ModelManager& ModelManager::Get()
{
	// TODO: 在此处插入 return 语句
}

void ModelManager::Init(ID3D11Device* device)
{
}

Model* ModelManager::CreateFromFile(std::string_view filename)
{
	return nullptr;
}

Model* ModelManager::CreateFromFile(std::string_view name, std::string_view filename)
{
	return nullptr;
}

Model* ModelManager::CreateFromGeometry(std::string_view name, const GeometryData& data, bool isDynamic)
{
	return nullptr;
}

const Model* ModelManager::GetModel(std::string_view name) const
{
	return nullptr;
}

Model* ModelManager::GetModel(std::string_view name)
{
	return nullptr;
}
