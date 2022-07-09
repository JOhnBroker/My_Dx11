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
	// 公告板绘制
	void SetRenderBillboard(ID3D11DeviceContext* deviceContext, bool enableAlphaToCoverage);

	// 矩阵设置
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

	// 光照、材质和纹理相关设置
	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	void SetMaterial(const Material& material);
	void SetTexture(ID3D11ShaderResourceView* texture);
	void SetTextureArray(ID3D11ShaderResourceView* textures);
	void SetEyePos(const DirectX::XMFLOAT3& eyePos);

	// 状态开关设置
	void SetFogState(bool isOn);
	void SetFogColor(DirectX::XMVECTOR fogColor);
	void SetFogRange(float fogRange);
	void SetFogStart(float fogStart);

	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif