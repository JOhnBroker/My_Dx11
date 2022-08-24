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

	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTex;

	XMFLOAT4X4 m_View, m_Proj;
	UINT m_MsaaLevels = 1;
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

	pImpl->m_pEffectHelper->SetBinaryCacheDirectory(L"HLSL\\Cache\\", true);

	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	D3D_SHADER_MACRO defines[] = {
		{"MSAA_SAMPLES","1"},
		{nullptr,nullptr}
	};

	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxVS", L"HLSL\\36\\Skybox.hlsl", device,
		"SkyboxVS", "vs_5_0", defines, blob.ReleaseAndGetAddressOf()));
	HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTex.GetAddressOf()));

	int msaaSamples = 1;
	while (msaaSamples <= 8)
	{
		std::string msaaSamplesStr = std::to_string(msaaSamples);
		defines[0].Definition = msaaSamplesStr.c_str();
		std::string shaderName = "Skybox_" + msaaSamplesStr + "xMSAA_PS";
		HR(pImpl->m_pEffectHelper->CreateShaderFromFile(shaderName, L"HLSL\\36\\Skybox.hlsl", device,
			"SkyboxPS", "ps_5_0", defines));

		std::string passName = "Skybox_" + msaaSamplesStr + "xMSAA";

		EffectPassDesc passDesc;
		passDesc.nameVS = "SkyboxVS";
		passDesc.namePS = shaderName.c_str();
		HR(pImpl->m_pEffectHelper->AddEffectPass(passName, device, &passDesc));
		{
			auto pPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
			pPass->SetRasterizerState(RenderStates::RSNoCull.Get());
		}

		msaaSamples <<= 1;
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
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSAnistropicClamp16x.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosNormalTex.Get(), "SkyboxEffect.VertexPosLayout");
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
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_SkyboxTexture", tm.GetTexture(str));
}

MeshDataInput SkyBoxEffect::GetInputData(const MeshData& meshData)
{
	MeshDataInput input;
	input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
	input.topology = pImpl->m_CurrTopology;
	input.pVertexBuffers =
	{
		meshData.m_pVertices.Get(),
		meshData.m_pNormals.Get(),
		meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
	};
	input.strides = { 12,12,8 };
	input.offsets = { 0,0,0 };
	input.pIndexBuffer = meshData.m_pIndices.Get();
	input.indexCount = meshData.m_IndexCount;
	return input;
}

void SkyBoxEffect::SetRenderDefault()
{
	std::string passName = "Skybox_" + std::to_string(pImpl->m_MsaaLevels) + "xMSAA";
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTex.Get();
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void SkyBoxEffect::SetRenderGS()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("GenerateSkybox");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTex;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void SkyBoxEffect::SetDepthTexture(ID3D11ShaderResourceView* depthTexture)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DepthTexture", depthTexture);
}

void SkyBoxEffect::SetLitTexture(ID3D11ShaderResourceView* litTexture)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_LitTexture", litTexture);
}

void SkyBoxEffect::SetMsaaSamples(UINT msaaSamples)
{
	pImpl->m_MsaaLevels = msaaSamples;
}

void SkyBoxEffect::SetViewProjMatrixs(DirectX::FXMMATRIX VP, int idx)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProjs")->SetRaw(&VP, sizeof(VP) * idx, sizeof(VP));
}

void SkyBoxEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX VP = XMLoadFloat4x4(&pImpl->m_View) * XMLoadFloat4x4(&pImpl->m_Proj);
	VP = XMMatrixTranspose(VP);

	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&VP);
	pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
