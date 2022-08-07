#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>	// 必须晚于Effects.h和d3dUtil.h包含
#include <DXTrace.h>
#include <Vertex.h>
#include <TextureManager.h>
#include <ModelManager.h>
#include "LightHelper.h"

using namespace DirectX;
# pragma warning(disable: 26812)

//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl
{
public:
	// 必须显式指定
	Impl() {}
	~Impl() = default;

public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;
	ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTangentTexLayout;

	XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};

	bool m_NormalmapEnabled = false;
};

//
// BasicEffect
//

namespace
{
	// BasicEffect单例
	static BasicEffect* g_pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
	if (g_pInstance)
		throw std::exception("BasicEffect is a singleton!");
	g_pInstance = this;
	pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
}

BasicEffect::BasicEffect(BasicEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

BasicEffect& BasicEffect::operator=(BasicEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

BasicEffect& BasicEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("BasicEffect needs an instance!");
	return *g_pInstance;
}


bool BasicEffect::InitAll(ID3D11Device* device)
{
	if (!device)
		return false;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	// 创建顶点着色器
	pImpl->m_pEffectHelper->CreateShaderFromFile("BasicVS", L"HLSL/Basic_VS.cso", device, "VS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf());
	pImpl->m_pEffectHelper->CreateShaderFromFile("OITRenderVS", L"HLSL/OIT_Render_VS.cso", device, "VS", "vs_5_0");
	// 创建顶点输入布局
	HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	// 创建像素着色器
	pImpl->m_pEffectHelper->CreateShaderFromFile("BasicPS", L"HLSL/Basic_PS.cso", device);
	pImpl->m_pEffectHelper->CreateShaderFromFile("OITRenderPS", L"HLSL/OIT_Render_PS.cso", device);
	pImpl->m_pEffectHelper->CreateShaderFromFile("OITStorePS", L"HLSL/OIT_Store_PS.cso", device);
	
	// 创建通道
	EffectPassDesc passDesc;
	passDesc.nameVS = "BasicVS";
	passDesc.namePS = "BasicPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Basic", device, &passDesc));

	passDesc.nameVS = "BasicVS";
	passDesc.namePS = "OITStorePS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("OITStore", device, &passDesc));
	{
		auto pPass = pImpl->m_pEffectHelper->GetEffectPass("OITStore");
		pPass->SetRasterizerState(RenderStates::RSNoCull.Get());
		pPass->SetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
		pPass->SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
	}

	passDesc.nameVS = "OITRenderVS";
	passDesc.namePS = "OITRenderPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("OITRender", device, &passDesc));

	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamLinearWrap", RenderStates::SSLinearWrap.Get());
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamPointClamp", RenderStates::SSPointClamp.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
#endif
	pImpl->m_pEffectHelper->SetDebugObjectName("BasicEffect");

	return true;
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV BasicEffect::SetViewMatrix(FXMMATRIX V)
{
	XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV BasicEffect::SetProjMatrix(FXMMATRIX P)
{
	XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void BasicEffect::SetMaterial(const Material& material)
{
	TextureManager& tm = TextureManager::Get();

	PhongMaterial phongMat{};
	phongMat.ambient = material.Get<XMFLOAT4>("$AmbientColor");
	phongMat.diffuse = material.Get<XMFLOAT4>("$DiffuseColor");
	phongMat.diffuse.w = material.Get<float>("$Opacity");
	phongMat.specular = material.Get<XMFLOAT4>("$SpecularColor");
	phongMat.specular.w = material.Has<float>("$SpecularFactor") ? material.Get<float>("$SpecularFactor") : 1.0f;
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Material")->SetRaw(&phongMat);

	auto pStr = material.TryGet<std::string>("$Diffuse");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", pStr ? tm.GetTexture(*pStr) : nullptr);

	XMMATRIX TexTransform = XMMatrixIdentity();
	if (material.Has<XMFLOAT2>("$TexScale")) 
	{
		auto& scale = material.Get<DirectX::XMFLOAT2>("$TexScale");
		TexTransform.r[0] *= scale.x;
		TexTransform.r[1] *= scale.y;
	}
	if (material.Has<XMFLOAT2>("$TexOffset")) 
	{
		auto& offset = material.Get<DirectX::XMFLOAT2>("$TexOffset");
		TexTransform.r[3] += XMVectorSet(offset.x, offset.y, 0.0f, 0.0f);
	}
	TexTransform = XMMatrixTranspose(TexTransform);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_TexTransform")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&TexTransform));
}

MeshDataInput BasicEffect::GetInputData(const MeshData& meshData)
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

void BasicEffect::SetDirLight(uint32_t pos, const DirectionalLight& dirLight)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_DirLight")->SetRaw(&dirLight, sizeof(dirLight) * pos, sizeof(dirLight));
}

void BasicEffect::SetPointLight(uint32_t pos, const PointLight& pointLight)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PointLight")->SetRaw(&pointLight, sizeof(pointLight) * pos, sizeof(pointLight));
}

void BasicEffect::SetSpotLight(uint32_t pos, const SpotLight& spotLight)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_SpotLight")->SetRaw(&spotLight, sizeof(spotLight) * pos, sizeof(spotLight));
}

void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EyePosW")->SetFloatVector(3, reinterpret_cast<const float*>(&eyePos));
}

void BasicEffect::SetReflectionEnabled(bool enabled)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ReflectionEnabled")->SetSInt(enabled);
}

void BasicEffect::SetRefractionEnabled(bool enabled)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_RefractionEnabled")->SetSInt(enabled);
}

void BasicEffect::SetRefractionEta(float eta)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Eta")->SetFloat(eta);
}

void BasicEffect::SetFogState(bool enabled)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogEnabled")->SetSInt(enabled);
}

void BasicEffect::SetFogStart(float fogStart)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogStart")->SetFloat(fogStart);
}

void BasicEffect::SetFogColor(const DirectX::XMFLOAT4& fogColor)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogColor")->SetFloatVector(4, reinterpret_cast<const float*>(&fogColor));
}

void BasicEffect::SetFogRange(float fogRange)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogRange")->SetFloat(fogRange);
}

void BasicEffect::SetTextureCube(ID3D11ShaderResourceView* textureCube)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_TexCube", textureCube);
}

void BasicEffect::SetWavesStates(bool enabled, float gridSpatialStep)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_GridSpatialStep")->SetFloat(gridSpatialStep);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WavesEnabled")->SetSInt(enabled);
}

void BasicEffect::SetRenderDefault()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Basic");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_NormalmapEnabled = false;
	pImpl->m_pCurrEffectPass->SetRasterizerState(nullptr);
	pImpl->m_pCurrEffectPass->SetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pImpl->m_pCurrEffectPass->SetDepthStencilState(nullptr, 0);
}

void BasicEffect::SetRenderWithNormalMap()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("NormalMap");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTangentTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_NormalmapEnabled = true;
}

void BasicEffect::SetRenderTransparent()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Basic");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_pCurrEffectPass->SetRasterizerState(RenderStates::RSNoCull.Get());
	pImpl->m_pCurrEffectPass->SetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
	pImpl->m_pCurrEffectPass->SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
}

void BasicEffect::ClearOITBuffers(
	ID3D11DeviceContext* deviceContext,
	ID3D11UnorderedAccessView* flBuffer,
	ID3D11UnorderedAccessView* startOffsetBuffer)
{
	uint32_t magicValue[1] = { (uint32_t)-1 };
	deviceContext->ClearUnorderedAccessViewUint(flBuffer, magicValue);
	deviceContext->ClearUnorderedAccessViewUint(startOffsetBuffer, magicValue);
}

void BasicEffect::SetRenderOITStorage(
	ID3D11UnorderedAccessView* flBuffer,
	ID3D11UnorderedAccessView* startOffsetBuffer,
	uint32_t renderTargetWidth)
{
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout.Get();
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("OITStore");

	uint32_t initCount[1] = { 0 };
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_FLBufferRW", flBuffer, initCount);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_StartOffsetBufferRW", startOffsetBuffer, initCount);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FrameWidth")->SetUInt(renderTargetWidth);
}

void BasicEffect::RenderOIT(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* flBuffer, 
	ID3D11ShaderResourceView* startOffsetBuffer, 
	ID3D11ShaderResourceView* input, 
	ID3D11RenderTargetView* output,
	const D3D11_VIEWPORT& vp)
{
	// 会清空之前设置在OM的UAVs
	deviceContext->OMSetRenderTargets(1, &output, nullptr);

	deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_BackGround", input);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_FLBuffer", flBuffer);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_StartOffsetBuffer", startOffsetBuffer);
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("OITRender");
	pImpl->m_pCurrEffectPass->Apply(deviceContext);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetViewports(1, &vp);

	deviceContext->Draw(3, 0);

	// 清空
	ID3D11ShaderResourceView* nullSRVs[]{ nullptr };
	deviceContext->PSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_BackGround"), 1, nullSRVs);
	deviceContext->PSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_FLBuffer"), 1, nullSRVs);
	deviceContext->PSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_StartOffsetBuffer"), 1, nullSRVs);
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void BasicEffect::SetTextureDisplacement(ID3D11ShaderResourceView* textureDisplacement)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DisplacementMap", textureDisplacement);
}

void BasicEffect::DrawInstanced(ID3D11DeviceContext* deviceContext, Buffer& instancedBuffer, const GameObject& object, uint32_t numObject)
{
	//deviceContext->IASetInputLayout(pImpl->m_pInstancePosNormalTexLayout.Get());
	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//auto pPass = pImpl->m_pEffectHelper->GetEffectPass("BasicInstance");
	//
	//XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
	//XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);
	//XMMATRIX VP = V * P;
	//VP = XMMatrixTranspose(VP);
	//pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&VP);
	//
	//const Model* pModel = object.GetModel();
	//size_t sz = pModel->meshdatas.size();
	//for (size_t i = 0; i < sz; ++i)
	//{
	//	SetMaterial(pModel->materials[pModel->meshdatas[i].m_MaterialIndex]);
	//	pPass->Apply(deviceContext);
	//
	//	MeshDataInput input = GetInputData(pModel->meshdatas[i]);
	//	input.pVertexBuffers.back() = instancedBuffer.GetBuffer();
	//	deviceContext->IASetVertexBuffers(0, (uint32_t)input.pVertexBuffers.size(),
	//		input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());
	//	deviceContext->IASetIndexBuffer(input.pIndexBuffer, input.indexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
	//
	//	deviceContext->DrawIndexedInstanced(input.indexCount, numObject, 0, 0, 0);
	//}
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX W = XMLoadFloat4x4(&pImpl->m_World);
	XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
	XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);

	XMMATRIX VP = V * P;
	XMMATRIX WInvT = XMath::InverseTranspose(W);

	W = XMMatrixTranspose(W);
	VP = XMMatrixTranspose(VP);
	WInvT = XMMatrixTranspose(WInvT);

	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (FLOAT*)&WInvT);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&VP);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (FLOAT*)&W);

	if (pImpl->m_pCurrEffectPass)
	{
		pImpl->m_pCurrEffectPass->Apply(deviceContext);
	}
}