#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>
#include <DXTrace.h>
#include <Vertex.h>
#include <TextureManager.h>

using namespace DirectX;

class SkyBoxEffect::Impl
{
public:
	Impl()
	{
		XMStoreFloat4x4(&m_View, XMMatrixIdentity());
		XMStoreFloat4x4(&m_Proj, XMMatrixIdentity());
	}
	~Impl() = default;
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	std::unique_ptr<EffectHelper> m_pEffectHelper;
	std::shared_ptr<IEffectPass> m_pCurrEffectPass;
	ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ComPtr<ID3D11InputLayout> m_pVertexPosLayout;

	XMFLOAT4X4 m_View, m_Proj;
};

namespace
{
	static SkyBoxEffect* g_pInstance = nullptr;
}

SkyBoxEffect::SkyBoxEffect()
{
	if (g_pInstance)
	{
		throw std::exception("SkyboxEffect is a singleton!");
	}
	g_pInstance = this;
	pImpl = std::make_unique<SkyBoxEffect::Impl>();
}

SkyBoxEffect::~SkyBoxEffect()
{
}

SkyBoxEffect::SkyBoxEffect(SkyBoxEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

SkyBoxEffect& SkyBoxEffect::operator=(SkyBoxEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

SkyBoxEffect& SkyBoxEffect::Get()
{
	if (!g_pInstance)
	{
		throw std::exception("SkyboxEffect needs an instance!");
	}
	return *g_pInstance;
}

bool SkyBoxEffect::InitAll(ID3D11Device* device)
{
	if (!device)
		return false;
	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");
	pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxVS", L"HLSL\\SkyBox_VS.cso", device, "VS",
		"vs_5_0", nullptr, blob.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPos::GetInputLayout(), ARRAYSIZE(VertexPos::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosLayout.ReleaseAndGetAddressOf()));
	//HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxGSVS", L"HLSL\\SkyBoxGS_VS.cso", device, "VS",
	//	"vs_5_0", nullptr, blob.ReleaseAndGetAddressOf()));

	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxPS", L"HLSL\\SkyBox_PS.cso", device, "PS"));
	//HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxGSPS", L"HLSL\\SkyBoxGS_PS.cso", device, "PS"));
	//HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxGS", L"HLSL\\SkyBox_GS.cso", device, "GS"));

	EffectPassDesc passDesc;
	passDesc.nameVS = "SkyboxVS";
	passDesc.namePS = "SkyboxPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Skybox", device, &passDesc))
	{
		auto pPass = pImpl->m_pEffectHelper->GetEffectPass("Skybox");
		pPass->SetRasterizerState(RenderStates::RSNoCull.Get());
		pPass->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
	}

	//passDesc.nameVS = "SkyboxGSVS";
	//passDesc.nameGS = "SkyboxGS";
	//passDesc.namePS = "SkyboxGSPS";
	//HR(pImpl->m_pEffectHelper->AddEffectPass("GenerateSkybox", device, &passDesc))
	//{
	//	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("GenerateSkybox");
	//	pPass->SetRasterizerState(RenderStates::RSNoCull.Get());
	//	pPass->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
	//}
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosLayout.Get(), "SkyboxEffect.VertexPosLayout");
#endif

	pImpl->m_pEffectHelper->SetDebugObjectName("SkyboxEffect");
	return true;
}

void XM_CALLCONV SkyBoxEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	UNREFERENCED_PARAMETER(W);
}

void XM_CALLCONV SkyBoxEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV SkyBoxEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void SkyBoxEffect::SetMaterial(const Material& material)
{
	TextureManager& tm = TextureManager::Get();
	const std::string& str = material.Get<std::string>("$Skybox");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_TexCube", tm.GetTexture(str));
}

MeshDataInput SkyBoxEffect::GetInputData(const MeshData& meshData)
{
	MeshDataInput input;
	input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
	input.topology = pImpl->m_CurrTopology;
	input.pVertexBuffers =
	{
		meshData.m_pVertices.Get()
	};
	input.strides = { 12 };
	input.offsets = { 0 };
	input.pIndexBuffer = meshData.m_pIndices.Get();
	input.indexCount = meshData.m_IndexCount;
	return input;
}

void SkyBoxEffect::SetRenderDefault()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Skybox");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void SkyBoxEffect::SetRenderGS()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("GenerateSkybox");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void SkyBoxEffect::SetViewProjMatrixs(DirectX::FXMMATRIX VP, int idx)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProjs")->SetRaw(&VP, sizeof(VP) * idx, sizeof(VP));
}

void SkyBoxEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
	XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);
	V.r[3] = g_XMIdentityR3;
	XMMATRIX VP = V * P;

	VP = XMMatrixTranspose(VP);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&VP);
	pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
