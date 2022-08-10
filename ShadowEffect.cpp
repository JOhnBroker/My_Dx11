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
	Impl() {};
	~Impl() = default;

public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;
	ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};

};

namespace
{
	static ShadowEffect* g_pInstance = nullptr;
}

ShadowEffect::ShadowEffect()
{
	if (g_pInstance)
	{
		throw std::exception("ShadowEffect is a singleton!");
	}
	g_pInstance = this;
	pImpl = std::make_unique<ShadowEffect::Impl>();
}

ShadowEffect::~ShadowEffect()
{
}

ShadowEffect::ShadowEffect(ShadowEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

ShadowEffect& ShadowEffect::operator=(ShadowEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

ShadowEffect& ShadowEffect::Get()
{
	if (!g_pInstance)
	{
		throw std::exception("ShadowEffect needs an instance!");
	}
	return *g_pInstance;
}

bool ShadowEffect::InitAll(ID3D11Device* device)
{
	if (!device)
		return false;
	if (!RenderStates::IsInit())
		throw std::exception("RenderState need to be initialized first!");

	pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	pImpl->m_pEffectHelper->SetBinaryCacheDirectory(L"HLSL\\Cache\\");

	// 创建顶点着色器
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("ShadowVS", L"HLSL\\Shadow.hlsl",
		device, "ShadowVS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf()));

	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("FullScreenTriangleTexcoordVS", L"HLSL\\FullScreenTriangle.hlsl",
		device, "FullScreenTriangleTexcoordVS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf()));

	// 创建像素着色器
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("ShadowPS", L"HLSL\\Shadow.hlsl", device, "ShadowPS", "ps_5_0"));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("DebugPS", L"HLSL\\Shadow.hlsl", device, "DebugPS", "ps_5_0"));

	// 创建通道Pass
	EffectPassDesc passDesc;
	passDesc.nameVS = "ShadowVS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("DepthOnly", device, &passDesc));
	pImpl->m_pEffectHelper->GetEffectPass("DepthOnly")->SetRasterizerState(RenderStates::RSShadow.Get());

	passDesc.namePS = "ShadowPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("DepthAlphaClip", device, &passDesc));
	pImpl->m_pEffectHelper->GetEffectPass("DepthAlphaClip")->SetRasterizerState(RenderStates::RSShadow.Get());

	passDesc.nameVS = "FullScreenTriangleTexcoordVS";
	passDesc.namePS = "DebugPs";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Debug", device, &passDesc));

	pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "ShadowEffect.VertexPosNormalTexLayout");
#endif
	pImpl->m_pEffectHelper->SetDebugObjectName("ShadowEffect");

	return true;
}

void XM_CALLCONV ShadowEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV ShadowEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV ShadowEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void ShadowEffect::SetMaterial(const Material& material)
{
	TextureManager& tm = TextureManager::Get();
	auto pStr = material.TryGet<std::string>("$Diffuse");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", pStr ? tm.GetTexture(*pStr) : nullptr);
}

MeshDataInput ShadowEffect::GetInputData(const MeshData& meshData)
{
	MeshDataInput input;
	input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
	input.topology = pImpl->m_CurrTopology;
	input.pVertexBuffers = {
		meshData.m_pVertices.Get(),
		meshData.m_pNormals.Get(),
		meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
	};
	input.strides = { 12,12,0 };
	input.offsets = { 0,0,0 };

	input.pIndexBuffer = meshData.m_pIndices.Get();
	input.indexCount = meshData.m_IndexCount;

	return input;
}

void ShadowEffect::SetRenderDepthOnly()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("DepthOnly");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void ShadowEffect::SetRenderAlphaClip()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("DepthAlphaClip");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void ShadowEffect::RenderDepthToTexture(
	ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView* input,
	ID3D11RenderTargetView* output,
	const D3D11_VIEWPORT& vp)
{
	deviceContext->IASetInputLayout(nullptr);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Debug");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", input);
	pImpl->m_pCurrEffectPass->Apply(deviceContext);
	deviceContext->OMSetRenderTargets(1, &output, nullptr);
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->Draw(3, 0);

	int slot = pImpl->m_pEffectHelper->MapShaderResourceSlot("g_DiffuseMap");
	input = nullptr;
	deviceContext->CSSetShaderResources(slot, 1, &input);
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void ShadowEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX MVP = XMLoadFloat4x4(&pImpl->m_World) * XMLoadFloat4x4(&pImpl->m_View) * XMLoadFloat4x4(&pImpl->m_Proj);
	MVP = XMMatrixTranspose(MVP);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (const float*)&MVP);

	pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
