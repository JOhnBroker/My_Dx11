#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>
#include <DXTrace.h>
#include <Vertex.h>
#include <TextureManager.h>
#include "HLSL/36/ShaderDefines.h"
using namespace DirectX;

class ForwardEffect::Impl
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
	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};

	UINT m_MsaaSamples = 1;
	int m_ShadowType = 0;
	int m_CascadeLevel = 0;
	int m_DerivativeOffset = 0;
	int m_CascadeBlend = 0;
	int m_CascadeSelection = 0;
	int m_PCFKernelSize = 1;
	int m_ShadowSize = 1024;
};

namespace {
	static ForwardEffect* g_pInstance = nullptr;
}

ForwardEffect::ForwardEffect()
{
	if (g_pInstance)
	{
		throw std::exception("ForwardEffect is a singleton!");
	}
	g_pInstance = this;
	pImpl = std::make_unique<ForwardEffect::Impl>();
}

ForwardEffect::~ForwardEffect()
{
}

ForwardEffect::ForwardEffect(ForwardEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

ForwardEffect& ForwardEffect::operator=(ForwardEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

ForwardEffect& ForwardEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("ForwardEffect needs an instance!");
	return *g_pInstance;
}

bool ForwardEffect::InitAll(ID3D11Device* device)
{
	if (!device)
		return false;
	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	pImpl->m_pEffectHelper->SetBinaryCacheDirectory(L"HLSL\\Cache", true);

	// 为了对每个着色器编译出最优版本，需要对同一个文件编译出80种版本的着色器。

	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	const char* numStrs[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
	D3D_SHADER_MACRO defines[] = {
		{ "SHADOW_TYPE", "0" },
		{ "CASCADE_COUNT_FLAG", "1" },
		{ "SELECT_CASCADE_BY_INTERVAL_FLAG", "0" },
		{ nullptr, nullptr }
	};

	// 顶点着色器
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("GeometryVS", L"HLSL\\36\\Rendering.hlsl",
		device, "GeometryVS", "vs_5_0", defines, blob.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.ReleaseAndGetAddressOf()));

	// 前三位代表
	// [阴影类别][级联级别][级联选择]
	std::string psName = "000_ForwardPS";
	std::string passName = "000_Forward";
	EffectPassDesc passDesc;

	passDesc.nameVS = "GeometryVS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("PreZ_Forward", device, &passDesc));

	for (int shadowType = 0; shadowType < 5; ++shadowType)
	{
		psName[0] = passName[0] = '0' + shadowType;
		defines[0].Definition = numStrs[shadowType];
		for (int cascadeCount = 1; cascadeCount <= 8; ++cascadeCount)
		{
			psName[1] = passName[1] = '0' + cascadeCount;
			defines[1].Definition = numStrs[cascadeCount];
			for (int intervalIdx = 0; intervalIdx < 2; ++intervalIdx)
			{
				psName[2] = passName[2] = '0' + intervalIdx;
				defines[2].Definition = numStrs[intervalIdx];

				HR(pImpl->m_pEffectHelper->CreateShaderFromFile(psName, L"HLSL\\36\\Rendering.hlsl", device, "ForwardPS", "ps_5_0", defines));

				passDesc.nameVS = "GeometryVS";
				passDesc.namePS = psName;
				HR(pImpl->m_pEffectHelper->AddEffectPass(passName, device, &passDesc));
			}
		}
	}

	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerDiffuse", RenderStates::SSAnistropicWrap16x.Get());
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerShadowCmp", RenderStates::SSShadowPCF.Get());
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerShadow", RenderStates::SSAnistropicClamp16x.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "ForwardEffect.VertexPosNormalTexLayout");
	pImpl->m_pEffectHelper->SetDebugObjectName("ForwardEffect");
#endif

	return true;
}

void ForwardEffect::SetLightBuffer(ID3D11ShaderResourceView* lightBuffer)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_light", lightBuffer);
}

void ForwardEffect::SetTileBuffer(ID3D11ShaderResourceView* tileBuffer)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Tilebuffer", tileBuffer);
}

void ForwardEffect::SetCameraNearFar(float nearZ, float farZ)
{
	float nearFar[4] = { nearZ,farZ };
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CameraNearFar")->SetFloatVector(4, nearFar);
}

void ForwardEffect::SetLightingOnly(bool enable)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_LightingOnly")->SetUInt(enable);
}

void ForwardEffect::SetFaceNormals(bool enable)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FaceNormals")->SetUInt(enable);
}

void ForwardEffect::SetVisualizeLightCount(bool enable)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_VisualizeLightCount")->SetUInt(enable);
}

void ForwardEffect::SetRenderDefault(bool reversedZ)
{
	std::string passName = "000_Forward";
	passName[0] = '0' + pImpl->m_ShadowType;
	passName[1] = '0' + pImpl->m_CascadeLevel;
	passName[2] = '0' + pImpl->m_CascadeSelection;
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
	pImpl->m_pCurrEffectPass->SetDepthStencilState(reversedZ ? RenderStates::DSSGreaterEqual.Get() : nullptr, 0);
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout.Get();
	pImpl->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void ForwardEffect::SetRenderPreZPass(bool reversedZ)
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("PreZ_Forward");
	pImpl->m_pCurrEffectPass->SetDepthStencilState(reversedZ ? RenderStates::DSSGreaterEqual.Get() : nullptr, 0);
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout.Get();
	pImpl->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void ForwardEffect::ComputeTiledLightCulling(ID3D11DeviceContext* deviceContext,
	ID3D11UnorderedAccessView* tileInfoBufferUAV,
	ID3D11ShaderResourceView* lightBufferSRV,
	ID3D11ShaderResourceView* depthBufferSRV)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex;
	depthBufferSRV->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.GetAddressOf()));
	D3D11_TEXTURE2D_DESC texDesc;
	pTex->GetDesc(&texDesc);

	UINT dims[2] = { texDesc.Width,texDesc.Height };
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FramebufferDimensions")->SetUIntVector(2, dims);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_TilebufferRW", tileInfoBufferUAV, 0);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_light", lightBufferSRV);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_GBufferTextures[3]", depthBufferSRV);

	std::string passName = "ComputeShaderTileForward_" + std::to_string(pImpl->m_MsaaSamples) + "xMSAA";
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
	pPass->Apply(deviceContext);

	// 调度
	pPass->Dispatch(deviceContext, texDesc.Width, texDesc.Height);

	//ID3D11UnorderedAccessView* nullUAV = nullptr;
	//ID3D11ShaderResourceView* nullSRV = nullptr;
	//int slot = pImpl->m_pEffectHelper->MapUnorderedAccessSlot("g_TilebufferRW");
	//deviceContext->CSSetUnorderedAccessViews(slot, 1, &nullUAV, nullptr);
	//slot = pImpl->m_pEffectHelper->MapShaderResourceSlot("g_light");
	//deviceContext->CSSetShaderResources(slot, 1, &nullSRV);
	//slot = pImpl->m_pEffectHelper->MapShaderResourceSlot("g_GBufferTextures[3]");
	//deviceContext->CSSetShaderResources(slot, 1, &nullSRV);
	tileInfoBufferUAV = nullptr;
	lightBufferSRV = nullptr;
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_TilebufferRW", tileInfoBufferUAV, 0);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_light", lightBufferSRV);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_GBufferTextures[3]", lightBufferSRV);
	pPass->Apply(deviceContext);
}

void ForwardEffect::Compute2Point5LightCulling(
	ID3D11DeviceContext* deviceContext,
	ID3D11UnorderedAccessView* tileInfoBufferUAV,
	ID3D11ShaderResourceView* lightBufferSRV,
	ID3D11ShaderResourceView* depthBufferSRV)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex;
	depthBufferSRV->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.GetAddressOf()));
	D3D11_TEXTURE2D_DESC texDesc;
	pTex->GetDesc(&texDesc);

	UINT dims[2] = { texDesc.Width,texDesc.Height };
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FramebufferDimensions")->SetUIntVector(2, dims);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_TilebufferRW", tileInfoBufferUAV, 0);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_light", lightBufferSRV);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_GBufferTextures[3]", depthBufferSRV);

	std::string passName = "PointLightCullingForward_" + std::to_string(pImpl->m_MsaaSamples) + "xMSAA";
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
	pPass->Apply(deviceContext);

	// 调度
	pPass->Dispatch(deviceContext, texDesc.Width, texDesc.Height);

	tileInfoBufferUAV = nullptr;
	lightBufferSRV = nullptr;
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_TilebufferRW", tileInfoBufferUAV, 0);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_light", lightBufferSRV);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_GBufferTextures[3]", lightBufferSRV);
	pPass->Apply(deviceContext);
}

void ForwardEffect::SetRenderWithTiledLightCulling()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("ForwardPlus");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout.Get();
	pImpl->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void XM_CALLCONV ForwardEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV ForwardEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV ForwardEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void ForwardEffect::SetMaterial(const Material& material)
{
	TextureManager& tm = TextureManager::Get();

	auto pStr = material.TryGet<std::string>("$Diffuse");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_TextureDiffuse", pStr ? tm.GetTexture(*pStr) : tm.GetNullTexture());
}

MeshDataInput ForwardEffect::GetInputData(const MeshData& meshData)
{
	MeshDataInput input;
	input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
	input.topology = pImpl->m_Topology;
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

void ForwardEffect::SetShadowType(int type)
{
	if (type > 4 || type < 0)
		return;
	pImpl->m_ShadowType = type;
}

void ForwardEffect::SetCascadeLevels(int cascadeLevels)
{
	pImpl->m_CascadeLevel = cascadeLevels;
}

void ForwardEffect::SetPCFDerivativesOffsetEnabled(bool enable)
{
	pImpl->m_DerivativeOffset = enable;
}

void ForwardEffect::SetCascadeBlendEnabled(bool enable)
{
	pImpl->m_CascadeBlend = enable;
}

void  ForwardEffect::SetCascadeIntervalSelectionEnabled(bool enable)
{
	pImpl->m_CascadeSelection = enable;
}

void  ForwardEffect::SetCascadeVisulization(bool enable)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_VisualizeCascades")->SetSInt(enable);
}

void ForwardEffect::Set16BitFormatShadow(bool enable)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_16BitShadow")->SetSInt(enable);
}

void  ForwardEffect::SetCascadeOffsets(const DirectX::XMFLOAT4 offsets[8])
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeOffset")->SetRaw(offsets);
}

void  ForwardEffect::SetCascadeScales(const DirectX::XMFLOAT4 scales[8])
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeScale")->SetRaw(scales);
}

void  ForwardEffect::SetCascadeFrustumsEyeSpaceDepths(const float depths[8])
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeFrustumsEyeSpaceDepthsData")->SetRaw(depths);
}

void  ForwardEffect::SetCascadeBlendArea(float blendArea)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeBlendArea")->SetFloat(blendArea);
}

void  ForwardEffect::SetPCFKernelSize(int size)
{
	int start = -size / 2;
    int end = size + start;
	pImpl->m_PCFKernelSize = size;
	float padding = (size / 2) / (float)pImpl->m_ShadowSize;
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PCFBlurForLoopStart")->SetSInt(start);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PCFBlurForLoopEnd")->SetSInt(end);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MinBorderPadding")->SetFloat(padding);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MaxBorderPadding")->SetFloat(1.0f - padding);
}

void  ForwardEffect::SetPCFDepthBias(float bias)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PCFDepthBias")->SetFloat(bias);
}

void  ForwardEffect::SetShadowSize(int size)
{
	pImpl->m_ShadowSize = size;
	float padding = 1.0f / (float)size;
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_TexelSize")->SetFloat(1.0f / size);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MinBorderPadding")->SetFloat(padding);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MaxBorderPadding")->SetFloat(1.0f - padding);
}

void XM_CALLCONV ForwardEffect::SetShadowViewMatrix(DirectX::FXMMATRIX ShadowView)
{
	XMMATRIX ShadowViewT = XMMatrixTranspose(ShadowView);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ShadowView")->SetFloatMatrix(4, 4, (const float *)&ShadowViewT);

}
void  ForwardEffect::SetShadowTextureArray(ID3D11ShaderResourceView* shadow)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_TextureShadow", shadow);
}

void ForwardEffect::SetPosExponent(float posExp)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EvsmPosExp")->SetFloat(posExp);
}

void ForwardEffect::SetNegExponent(float negExp)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EvsmNegExp")->SetFloat(negExp);
}

void ForwardEffect::SetLightBleedingReduction(float value)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_LightBleedingReduction")->SetFloat(value);
}

void ForwardEffect::SetCascadeSampler(ID3D11SamplerState* sampler)
{
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerShadow", sampler);
}

void ForwardEffect::SetMagicPower(float power)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MagicPower")->SetFloat(power);
}

void ForwardEffect::SetLightDir(const DirectX::XMFLOAT3& dir)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_LightDir")->SetFloatVector(3, (const float*)&dir);
}

void ForwardEffect::SetMsaaSamples(UINT msaaSamples)
{
	pImpl->m_MsaaSamples = msaaSamples;
}

void ForwardEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX W = XMLoadFloat4x4(&pImpl->m_World);
	XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
	XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);

	XMMATRIX WV = W * V;
    XMMATRIX WVP = W * V * P;
	XMMATRIX WInvT = XMath::InverseTranspose(W);

    W = XMMatrixTranspose(W);
	WV = XMMatrixTranspose(WV);
	WVP = XMMatrixTranspose(WVP);
	WInvT = XMMatrixTranspose(WInvT);

	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (FLOAT*)&WInvT);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&WVP);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldView")->SetFloatMatrix(4, 4, (FLOAT*)&WV);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (FLOAT*)&W);

	if (pImpl->m_pCurrEffectPass)
		pImpl->m_pCurrEffectPass->Apply(deviceContext);
}

