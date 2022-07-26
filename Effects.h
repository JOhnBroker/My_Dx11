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
#include <RenderStates.h>

#include <Buffer.h>

#include <GameObject.h>

class BasicEffect : public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	struct InstancedData
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 worldInvTranspose;
		DirectX::XMFLOAT4 color;
	};

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

	void SetTextureCube(ID3D11ShaderResourceView* textureCube);

	// 绘制实例
	void DrawInstanced(ID3D11DeviceContext* deviceContext, Buffer& buffer, const GameObject& object, uint32_t numObject);

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	void SetEyePos(const DirectX::XMFLOAT3& eyePos);
	void SetDiffuseColor(const DirectX::XMFLOAT4& color);

	void SetReflectionEnabled(bool enabled);
	void SetRefractionEnabled(bool enabled);
	void SetRefractionEta(float eta);

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


class SkyBoxEffect :public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	SkyBoxEffect();
	virtual ~SkyBoxEffect() override;

	SkyBoxEffect(SkyBoxEffect&& moveFrom)noexcept;
	SkyBoxEffect& operator=(SkyBoxEffect&& moveFrom)noexcept;

	//
	static SkyBoxEffect& Get();

	//
	bool InitAll(ID3D11Device* device);

	//
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	void SetRenderDefault();

	void Apply(ID3D11DeviceContext* deviceContext) override;
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif