#pragma once

#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <wrl/client.h>
#include <vector>
#include <DirectXCollision.h>

struct  ID3D11Buffer;

struct MeshData
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11Buffer> m_pVertices;
	ComPtr<ID3D11Buffer> m_pNormals;
	std::vector<ComPtr<ID3D11Buffer>> m_pTexcoordArrays;
	ComPtr<ID3D11Buffer> m_pTangents;
	ComPtr<ID3D11Buffer> m_pBitangents;
	ComPtr<ID3D11Buffer> m_pColors;

	ComPtr<ID3D11Buffer> m_pIndices;
	uint32_t m_VertexCount = 0;
	uint32_t m_IndexCount = 0;
	uint32_t m_MaterialIndex = 0;

	DirectX::BoundingBox m_BoundingBox;
	bool m_InFrustum = true;
};

#endif