#include "XUtil.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace DirectX;

void Model::CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename)
{
	using namespace Assimp;
	namespace fs = std::filesystem;

	model.materials.clear();
	model.meshdatas.clear();
	model.boundingbox = BoundingBox();

	Importer importer;
	auto pAssimpScene = importer.ReadFile(filename.data(), aiProcess_ConvertToLeftHanded |
		aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_ImproveCacheLocality);

	if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
	{
		model.meshdatas.resize(pAssimpScene->mNumMeshes);
		model.materials.resize(pAssimpScene->mNumMaterials);
		for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
		{
			auto& mesh = model.meshdatas[i];
			auto pAiMesh = pAssimpScene->mMeshes[i];
			uint32_t numVertices = pAiMesh->mNumVertices;

			CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
			D3D11_SUBRESOURCE_DATA initData{ nullptr,0,0 };
			// 位置
			if (pAiMesh->mNumVertices > 0)
			{
				initData.pSysMem = pAiMesh->mVertices;
				bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());

				BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
					(const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3));
				if (i == 0)
				{
					model.boundingbox = mesh.m_BoundingBox;
				}
				else
				{
					model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);
				}
			}
			// 法线
			if (pAiMesh->HasNormals())
			{
				initData.pSysMem = pAiMesh->mNormals;
				bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());
			}
			// 切线和副法线


		}
	}

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
