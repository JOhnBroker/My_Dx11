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

	// 默认状态来绘制
	void SetRenderDefault();
	// 透明混合绘制
	void SetRenderTransparent();

	// OIT pass1

	// 清空OIT缓冲区
	void ClearOITBuffers(
		ID3D11DeviceContext* deviceContext,
		ID3D11UnorderedAccessView* flBuffer,
		ID3D11UnorderedAccessView* startOffsetBuffer);

	// 顺序无关透明度存储
	void SetRenderOITStorage(
		ID3D11UnorderedAccessView* flBuffer,
		ID3D11UnorderedAccessView* startOffsetBuffer,
		uint32_t renderTargetWidth);

	// OIT pass2

	// 完成OIT渲染
	void RenderOIT(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* flBuffer,
		ID3D11ShaderResourceView* startOffsetBuffer,
		ID3D11ShaderResourceView* input,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

	void SetTextureDisplacement(ID3D11ShaderResourceView* textureDisplacement);

	void SetRenderWithNormalMap();

	void SetTextureCube(ID3D11ShaderResourceView* textureCube);

	// 绘制实例
	void DrawInstanced(ID3D11DeviceContext* deviceContext, Buffer& buffer, const GameObject& object, uint32_t numObject);

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(uint32_t pos, const DirectionalLight& dirLight);
	void SetPointLight(uint32_t pos, const PointLight& pointLight);
	void SetSpotLight(uint32_t pos, const SpotLight& spotLight);

	void SetEyePos(const DirectX::XMFLOAT3& eyePos);

	void SetReflectionEnabled(bool enabled);
	void SetRefractionEnabled(bool enabled);
	void SetRefractionEta(float eta);

	void SetFogState(bool enabled);
	void SetFogStart(float fogStart);
	void SetFogColor(const DirectX::XMFLOAT4& fogColor);
	void SetFogRange(float fogRange);

	void SetWavesStates(bool enabled, float gridSpatialStep = 0.0f);

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

class ShadowEffect :public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData 
{
public:
	ShadowEffect();
	virtual ~ShadowEffect() override;

	ShadowEffect(ShadowEffect&& moveFrom) noexcept;
	ShadowEffect& operator=(ShadowEffect&& moveFrom) noexcept;

	static ShadowEffect& Get();

	bool InitAll(ID3D11Device* device);

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	void SetRenderDepthOnly();

	void SetRenderAlphaClip();

	void RenderDepthToTexture(ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

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

	// 获取单例
	static SkyBoxEffect& Get();

	// 初始化所需资源
	bool InitAll(ID3D11Device* device);

	//IEffectTransform
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	void SetRenderDefault();
	void SetRenderGS();

	void SetViewProjMatrixs(DirectX::FXMMATRIX VP, int idx);

	void Apply(ID3D11DeviceContext* deviceContext) override;
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


class PostProcessEffect
{
public:
	PostProcessEffect();
	~PostProcessEffect();

	PostProcessEffect(PostProcessEffect&& moveForm) noexcept;
	PostProcessEffect& operator=(PostProcessEffect&& moveForm) noexcept;

	//
	static PostProcessEffect& Get();

	bool InitAll(ID3D11Device* device);

	// 将两张图片进行分量相乘
	// input2 为nullptr则直通
	void RenderComposite(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input1,
		ID3D11ShaderResourceView* input2,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

	// Sobel 滤波
	void ComputeSobel(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11UnorderedAccessView* output,
		uint32_t width, uint32_t height);

	// 高斯滤波
	void SetBlurKernelSize(int size);
	void SetBlurSigma(float sigma);

	void ComputeGaussianBlurX(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11UnorderedAccessView* output,
		uint32_t width, uint32_t height);
	void ComputeGaussianBlurY(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11UnorderedAccessView* output,
		uint32_t width, uint32_t height);

	// 渐变特效
	void RenderScreenFade(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp,
		float fadeAmount);

	// 小地图
	void SetVisibleRange(float range);
	void SetEyePos(const DirectX::XMFLOAT3& eyePos);
	void SetMinimapRect(const DirectX::XMFLOAT4& rect);// (Left, Front, Right, Back)
	void RenderMinimap(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif