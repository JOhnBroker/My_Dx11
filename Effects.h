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
//#include <LightHelper.h>
#include <RenderStates.h>
#include <Buffer.h>
#include <GameObject.h>

enum class RasterizerMode { Solid, Wireframe };

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
	// 带法线/位移贴图的绘制
	void SetRenderWithDisplacementMap();
	// 带法线贴图绘制
	void SetRenderWithNormalMap();

	// 给当前Pass设置光栅化模式
	void SetRasterizerMode(RasterizerMode mode);

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

	// 天空盒
	void SetTextureCube(ID3D11ShaderResourceView* textureCube);

	// 阴影
	void XM_CALLCONV SetShadowTransformMatrix(DirectX::FXMMATRIX S);
	void SetDepthBias(float bias);
	void SetTextureShadowMap(ID3D11ShaderResourceView* textureShadowMap);

	// SSAO
	void SetSSAOEnabled(bool enabled);
	void SetTextureAmbientOcclusion(ID3D11ShaderResourceView* textureAmbientOcclusion);

	// 曲面细分
	void SetHeightScale(float scale);
	void SetTessInfo(float maxTessDistance, float minTessDistance, float minTessFactor, float maxTessFactor);

	// 绘制实例
	void DrawInstanced(ID3D11DeviceContext* deviceContext, Buffer& buffer, const GameObject& object, uint32_t numObject);

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	// 设置其它参数

	//void SetDirLight(uint32_t pos, const DirectionalLight& dirLight);
	//void SetPointLight(uint32_t pos, const PointLight& pointLight);
	//void SetSpotLight(uint32_t pos, const SpotLight& spotLight);

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

	void SetRenderDepthOnly(bool enableAlphaClip = false);

	void SetRenderDepthOnlyWithDisplacement(bool enableAlphaClip = false);

	// 曲面细分
	void SetEyePos(const DirectX::XMFLOAT3& eyePos);
	void SetHeightScale(float scale);
	void SetTessInfo(float maxTessDistance, float minTessDistance, float minTessFactor, float maxTessFactor);

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

	// 设置深度图
	void SetDepthTexture(ID3D11ShaderResourceView* depthTexture);
	// 设置场景渲染图
	void SetLitTexture(ID3D11ShaderResourceView* litTexture);
	void SetFlatLitTexture(ID3D11ShaderResourceView* flatLitTexture, UINT width, UINT height);

	void SetMsaaSamples(UINT msaaSamples);

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


class SSAOEffect :public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	SSAOEffect();
	virtual ~SSAOEffect() override;

	SSAOEffect(SSAOEffect&& moveFrom) noexcept;
	SSAOEffect& operator=(SSAOEffect&& moveFrom) noexcept;

	static SSAOEffect& Get();

	bool InitAll(ID3D11Device* device);

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	// Pass1 绘制观察空间法向量和深度贴图
	void SetRenderNormalDepthMap(bool enableAlphaClip = false);

	void SetRenderNormalDepthMapWithDisplacement(bool enableAlphaClip = false);

	void SetRasterizerMode(RasterizerMode mode);

	// 曲面细分
	void SetEyePos(const DirectX::XMFLOAT3& eyePos);
	void SetHeightScale(float scale);
	void SetTessInfo(float maxTessDistance, float minTessDistance, float minTessFactor, float maxTessFactor);


	// Pass2 绘制SSAO图
	void SetTextureRandomVec(ID3D11ShaderResourceView* textureRandomVec);

	void SetOcclusionInfo(float radius, float fadeStart, float fadeEnd, float surfaceEpsilon);

	void SetFrustumFarPlanePoints(const DirectX::XMFLOAT4 farPlanePoints[3]);

	void SetOffsetVectors(const DirectX::XMFLOAT4 offsetVectors[14]);

	void RenderToSSAOTexture(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* normalDepth,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp,
		uint32_t sampleCount);

	// Pass3 对SSAO图进行双边滤波

	void SetBlurWeight(const float weight[11]);

	void SetBlurRadius(int radius);

	// 进行水平双边滤波，要求输入输出图像大小相同
	void BilateralBlurX(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11ShaderResourceView* normalDepth,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

	// 进行竖直双边滤波，要求输入输出图像大小相同
	void BilateralBlurY(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11ShaderResourceView* normalDepth,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

	// 绘制AO图到纹理
	void RenderAmbientOcclusionToTexture(ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* input,
		ID3D11RenderTargetView* output,
		const D3D11_VIEWPORT& vp);

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

class ParticleEffect :public IEffect
{
public:
	struct VertexParticle
	{
		DirectX::XMFLOAT3 initialPos;
		DirectX::XMFLOAT3 initialVel;
		DirectX::XMFLOAT2 size;
		float age;
		uint32_t type;
	};
	struct InputData
	{
		ID3D11InputLayout* pInputLayout;
		D3D11_PRIMITIVE_TOPOLOGY topology;
		uint32_t stride;
		uint32_t offset;
	};

public:
	ParticleEffect();
	virtual ~ParticleEffect() override;

	ParticleEffect(ParticleEffect&& moveFrom) noexcept;
	ParticleEffect& operator=(ParticleEffect&& moveFrom) noexcept;

	bool InitAll(ID3D11Device* device, std::wstring_view filename);

	// vertexCount为0时调用drawAuto
	void RenderToVertexBuffer(
		ID3D11DeviceContext* deviceContext,
		ID3D11Buffer* input,
		ID3D11Buffer* output,
		uint32_t vertexCount = 0);
	// 绘制粒子系统
	InputData SetRenderDefault();

	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

	void SetEyePos(const DirectX::XMFLOAT3& eyePos);

	void SetGameTime(float t);
	void SetTimeStep(float step);

	void SetEmitDir(const DirectX::XMFLOAT3& dir);
	void SetEmitPos(const DirectX::XMFLOAT3& pos);

	void SetEmitInterval(float t);
	void SetAliveTime(float t);

	void SetAcceleration(const DirectX::XMFLOAT3& accel);

	void SetTextureInput(ID3D11ShaderResourceView* textureInput);
	void SetTextureRandom(ID3D11ShaderResourceView* textureRandom);

	void SetRasterizerState(ID3D11RasterizerState* rasterizerState);
	void SetBlendState(ID3D11BlendState* blendState, const float blendFactor[4], uint32_t sampleMask);
	void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState, UINT stencilRef);

	void Apply(ID3D11DeviceContext* deviceContext)override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

class ForwardEffect :public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	ForwardEffect();
	virtual ~ForwardEffect() override;

	ForwardEffect(ForwardEffect&& moveFrom) noexcept;
	ForwardEffect& operator=(ForwardEffect&& moveFrom) noexcept;

	static ForwardEffect& Get();

	bool InitAll(ID3D11Device* device);

	// IEffectTransform

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	void SetMsaaSamples(UINT msaaSamples);

	void SetLightBuffer(ID3D11ShaderResourceView* lightBuffer);
	void SetTileBuffer(ID3D11ShaderResourceView* tileBuffer);
	void SetCameraNearFar(float nearZ, float farZ);
	
	void SetLightingOnly(bool enable);
	void SetFaceNormals(bool enable);
	void SetVisualizeLightCount(bool enable);
	
	// 默认状态来绘制
	void SetRenderDefault();
	// 进行 Pre-Z 通道绘制
	void SetRenderPreZPass();
	// 执行分块光照裁剪
	void ComputeTiledLightCulling(ID3D11DeviceContext* deviceContext,
		ID3D11UnorderedAccessView* tileInfoBufferUAV,
		ID3D11ShaderResourceView* lightBufferSRV,
		ID3D11ShaderResourceView* depthBufferSRV);
	// 根据裁剪后的光照数据绘制
	void SetRenderWithTiledLightCulling();

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;

};

class DeferredEffect :public IEffect, public IEffectTransform,
	public IEffectMaterial, public IEffectMeshData
{
public:
	DeferredEffect();
	virtual ~DeferredEffect() override;

	DeferredEffect(DeferredEffect&& moveFrom) noexcept;
	DeferredEffect& operator=(DeferredEffect&& moveFrom) noexcept;

	static DeferredEffect& Get();

	bool InitAll(ID3D11Device* device);

	// IEffectTransform

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

	void SetMaterial(const Material& material) override;

	MeshDataInput GetInputData(const MeshData& meshData) override;

	void SetMsaaSamples(UINT msaaSamples);

	void SetLightingOnly(bool enable);
	void SetFaceNormals(bool enable);
	void SetVisualizeLightCount(bool enable);
	void SetVisualizeShadingFreq(bool enable);

	void SetCameraNearFar(float nearZ, float farZ);
	
	// 绘制G缓冲区
	void SetRenderGBuffer();
	// 将法线G-Buffer 渲染到目标纹理
	void DebugNormalGBuffer(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rtv, ID3D11ShaderResourceView* normalGBuffer, D3D11_VIEWPORT viewport);
	// 将深度值梯度的G-Buffer 渲染到到目标纹理
	void DebugPosZGradGBuffer(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rtv, ID3D11ShaderResourceView* posZGradGBuffer, D3D11_VIEWPORT viewport);
	// 传统延迟渲染
	void ComputeLightingDefault(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* litBufferRTV, ID3D11DepthStencilView* depthBufferReadOnlyDSV, ID3D11ShaderResourceView* lightBufferSRV, ID3D11ShaderResourceView* GBuffers[4], D3D11_VIEWPORT viewport);
	// 执行分块光照裁剪
	void ComputeTiledLightCulling(ID3D11DeviceContext* deviceContext, ID3D11UnorderedAccessView* litFlatBufferUAV, ID3D11ShaderResourceView* lightBufferSRV, ID3D11ShaderResourceView* GBuffers[4]);
	
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;

};


#endif