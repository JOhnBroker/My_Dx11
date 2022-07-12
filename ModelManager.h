////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : ModelManager.h
//  Creator     : XJun ？
//  Create Date : 2022
//  Purpose     : 模型管理
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <d3d11_1.h>
#include <vector>
#include <wrl/client.h>
#include "WinMin.h"
#include "Geometry.h"
#include "Material.h"
#include "MeshData.h"

struct Model
{
	Model();
	~Model();
	Model(Model&) = delete;
	Model& operator=(Model&) = delete;
	Model(Model&&) = default;
	Model& operator=(Model&&) = default;

	std::vector<Material> materials;
	std::vector<MeshData> meshdatas;
	DirectX::BoundingBox boundingbox;
	static void CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename);
	static void CreateFromGeometry(Model& model, ID3D11Device* device, const GeometryData& data, bool isDynamic = false);

	void SetDebugObjectName(std::string_view name);
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager(ModelManager&&) = default;
	ModelManager& operator=(ModelManager&&) = default;

	static ModelManager& Get();
	void Init(ID3D11Device* device);
	Model* CreateFromFile(std::string_view filename);
	Model* CreateFromFile(std::string_view name, std::string_view filename);
	Model* CreateFromGeometry(std::string_view name, const GeometryData& data, bool isDynamic = false);

	const Model* GetModel(std::string_view name)const;
	Model* GetModel(std::string_view name);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	std::unordered_map<size_t, Model>m_Models;
};

#endif