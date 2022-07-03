#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include <LightHelper.h>
#include <RenderStates.h>

class IEffect
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	IEffect() = default;
	virtual ~IEffect() = default;

	//不允许拷贝，允许移动
	IEffect(const IEffect&) = delete;
	IEffect& operator=(const IEffect&) = delete;
	IEffect(IEffect&&) = default;
	IEffect& operator=(IEffect&&) = default;

	virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;
};

class BasicEffect : public IEffect
{
public:
	BasicEffect();
	virtual ~BasicEffect() override;

	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	static BasicEffect& Get();

	bool InitAll(ID3D11Device* device);

	// 渲染模式的变更
	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext);
	// Alpha 混合绘制
	void SetRenderAlphaBlend(ID3D11DeviceContext* deviceContext);
	// 无二次混合
	void SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext);
	// 仅写入模板值
	void SetWriteStencilOnly(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 对指定模板值的区域进行绘制，采用默认状态
	void SetRenderDefaultWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 对指定模板值的区域进行绘制，采用Alpha混合
	void SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 2D默认状态绘制
	void Set2DRenderDefault(ID3D11DeviceContext* deviceContext);
	// 2D混合绘制
	void Set2DRenderAlphaBlend(ID3D11DeviceContext* deviceContext);

	// 矩阵设置
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

	void XM_CALLCONV SetReflectionMatrix(DirectX::FXMMATRIX R);
	void XM_CALLCONV SetShadowMatrix(DirectX::FXMMATRIX S);
	void XM_CALLCONV SetRefShadowMatrix(DirectX::FXMMATRIX RefS);

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;
	
	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& dirLight);
	void SetSpotLight(size_t pos, const SpotLight& dirLight);

	void SetMaterial(const Material& material);
	void SetTexture(ID3D11ShaderResourceView* textuer);
	void SetEyePos(const DirectX::XMFLOAT3& eyePos);

	// 状态开关设置
	void SetReflectionState(bool isOn);
	void SetShadowState(bool isOn);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif