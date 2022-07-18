////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : Effects.h
//  Creator     : XJun ？
//  Create Date : 2022
//  Purpose     : 简易特效管理框架
//
////////////////////////////////////////////////////////////////////////////////


#ifndef EFFECTS_H
#define EFFECTS_H

#include <IEffect.h>
#include <Material.h>
#include <MeshData.h>
#include <LightHelper.h>

class BasicEffect : public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	BasicEffect();
	virtual ~BasicEffect() override;

	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	static BasicEffect& Get();

	bool InitAll(ID3D11Device* device);

	// IEffectTransform
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	// IEffectMaterial
	void SetMaterial(const Material& material) override;

	// IEffectMeshData
	MeshDataInput GetInputData(const MeshData& meshData) override;

	// BasicEffect

	void SetRenderDefault();

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	void SetEyePos(const DirectX::XMFLOAT3& eyePos);

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif