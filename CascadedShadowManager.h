////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : CascadedShadowManager.h
//  Creator     : XJun ？
//  Create Date : 2022
//  Purpose     : 级联阴影管理
//
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef CASCADED_SHADOW_MANAGER_H
#define CASCADED_SHADOW_MANAGER_H

#include "WinMin.h"
#include <d3d11_1.h>
#include <DirectXCollision.h>
#include <wrl/client.h>
#include <memory>
#include <CameraController.h>
#include <Texture2D.h>

enum class ShadowType
{
	ShadowType_CSM,
	ShadowType_VSM,
	ShadowType_ESM,
	ShadowType_EVSM2,
	ShadowType_EVSM4
};

enum class CascadeSelection
{
	CascadeSelection_Map,
	CascadeSelection_Interval
};

enum class CameraSelection
{
	CameraSelection_Eye,
	CameraSelection_Light,
	CameraSelection_Cascade1,
	CameraSelection_Cascade2,
	CameraSelection_Cascade3,
	CameraSelection_Cascade4,
	CameraSelection_Cascade5,
	CameraSelection_Cascade6,
	CameraSelection_Cascade7,
	CameraSelection_Cascade8
};

enum class FitNearFar
{
	FitNearFar_ZeroOne,
	FitNearFar_CascadeAABB,
	FitNearFar_SceneAABB,
	FitNearFar_SceneAABB_Intersection,
};

enum class FitProjection
{
	FitProjection_ToCascade,
	FitProjection_ToScene
};

class CascadedShadowManager
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CascadedShadowManager() = default;
	~CascadedShadowManager() = default;
	CascadedShadowManager(const CascadedShadowManager&) = delete;
	CascadedShadowManager& operator=(const CascadedShadowManager&) = delete;
	CascadedShadowManager(CascadedShadowManager&&) = default;
	CascadedShadowManager& operator=(CascadedShadowManager&&) = default;

	HRESULT InitResource(ID3D11Device* device);

	void UpdateFrame(const Camera& viewerCamera,
		const Camera& lightCamera,
		const DirectX::BoundingBox& sceneBoundingBox);

	ID3D11RenderTargetView* GetCascadeRenderTargetView(size_t cascadeIndex)const { return m_pCSMTextureArray->GetRenderTarget(cascadeIndex); }
	ID3D11ShaderResourceView* GetCascadesOutput() const { return m_pCSMTextureArray->GetShaderResource(); }
	ID3D11ShaderResourceView* GetCascadeOutput(size_t cascadeIndex)const { return m_pCSMTextureArray->GetShaderResource(cascadeIndex); }

	ID3D11DepthStencilView* GetDepthBufferDSV()const { return m_pCSMDepthBuffer->GetDepthStencil(); }
	ID3D11ShaderResourceView* GetDepthBufferSRV()const { return m_pCSMDepthBuffer->GetShaderResource(); }

	ID3D11RenderTargetView* GetTempTextureRTV()const { return m_pCSMTempTexture->GetRenderTarget(); }
	ID3D11ShaderResourceView* GetTempTextureOutput()const { return m_pCSMTempTexture->GetShaderResource(); }

	const float* GetCascadePartitions()const { return m_CascadePartitionsFrustum; }
	void GetCascadePartitions(float output[8])const { memcpy_s(output, sizeof(m_CascadePartitionsFrustum), m_CascadePartitionsFrustum, sizeof(m_CascadePartitionsFrustum)); }
	DirectX::XMMATRIX GetShadowProjectionXM(size_t cascadeIndex)const { return XMLoadFloat4x4(&m_ShadowProj[cascadeIndex]); }
	DirectX::BoundingBox GetShadowAABB(size_t cascadeIndex)const { return m_ShadowProjBoundingBox[cascadeIndex]; }
	DirectX::BoundingOrientedBox GetShadowOBB(size_t cascadeIndex)const {
		DirectX::BoundingOrientedBox obb;
		DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, GetShadowAABB(cascadeIndex));
		return obb;
	}
	const D3D11_VIEWPORT& GetShadowViewport() const { return m_ShadowViewport; }

public:
	// 级联相关的配置
	ShadowType	m_ShadowType = ShadowType::ShadowType_CSM;
	int			m_ShadowSize = 1024;
	int			m_ShadowBits = 4;
	int			m_CascadeLevels = 4;
	bool		m_GenerateMips = false;

	float		m_CascadePartitionsPercentage[8]
	{
		0.04f,0.10f,0.25f,1.0f,1.0f,1.0f,1.0f,1.0f
	};
	int m_BlurKernelSize = 5;
	float m_GaussianBlurSigma = 3.0f;
	float m_BlendBetweenCascadesRange = 0.2f;
	float m_PCFDepthBias = 0.001f;
	float m_LightBleedingReduction = 0.8f;
	float m_MagicPower = 160.0f;
	float m_PosExp = 5.0f;
	float m_NegExp = 5.0f;

	bool m_DerivativeBasedOffset = false;
	bool m_BlendBetweenCascades = true;

	bool m_FixedSizeFrustumAABB = true;
	bool m_MoveLightTexelSize = true;

	CameraSelection		m_SelectedCamera = CameraSelection::CameraSelection_Eye;
	FitProjection		m_SelectedCascadesFit = FitProjection::FitProjection_ToCascade;
	FitNearFar			m_SelectedNearFarFit = FitNearFar::FitNearFar_SceneAABB_Intersection;
	CascadeSelection	m_SelectedCascadeSelection = CascadeSelection::CascadeSelection_Map;

private:
	void XM_CALLCONV ComputeNearAndFar(float& outNearPlane, float& outFarPlane,
		DirectX::FXMVECTOR lightCameraOrthographicMinVec,
		DirectX::FXMVECTOR lightCameraOrthographicMaxVec,
		DirectX::XMVECTOR pointsInCameraView[]);

private:
	float m_CascadePartitionsFrustum[8]{};
	DirectX::XMFLOAT4X4 m_ShadowProj[8]{};
	DirectX::BoundingBox m_ShadowProjBoundingBox[8]{};
	D3D11_VIEWPORT m_ShadowViewport{};

	std::unique_ptr<Texture2DArray> m_pCSMTextureArray;
	std::unique_ptr<Texture2D> m_pCSMTempTexture;
	std::unique_ptr<Depth2D> m_pCSMDepthBuffer;
};

#endif