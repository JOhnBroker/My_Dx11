#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>
#include <DXTrace.h>
#include <Vertex.h>
#include <TextureManager.h>
#include <ModelManager.h>
#include "LightHelper.h"

using namespace DirectX;

class ShadowEffect::Impl
{
public:
	Impl();
	~Impl() = default;

public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;
	ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	XMFLOAT4 m_World{}, m_View{}, m_Proj{};

};

namespace 
{
	static ShadowEffect* g_pInstance = nullptr;
}

ShadowEffect::ShadowEffect()
{
}

ShadowEffect::~ShadowEffect()
{
}

ShadowEffect::ShadowEffect(ShadowEffect&& moveFrom) noexcept
{
}

ShadowEffect& ShadowEffect::operator=(ShadowEffect&& moveFrom) noexcept
{
	// TODO: 在此处插入 return 语句
}

ShadowEffect& ShadowEffect::Get()
{
	// TODO: 在此处插入 return 语句
}

bool ShadowEffect::InitAll(ID3D11Device* device)
{
	return false;
}

void XM_CALLCONV ShadowEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	return void ();
}

void XM_CALLCONV ShadowEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	return void ();
}

void XM_CALLCONV ShadowEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	return void ();
}

void ShadowEffect::SetMaterial(const Material& material)
{
}

MeshDataInput ShadowEffect::GetInputData(const MeshData& meshData)
{
	return MeshDataInput();
}

void ShadowEffect::SetRenderDepthOnly()
{
}

void ShadowEffect::SetRenderAlphaClip()
{
}

void ShadowEffect::RenderDepthToTexture(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, const D3D11_VIEWPORT& vp)
{
}

void ShadowEffect::Apply(ID3D11DeviceContext* deviceContext)
{
}
