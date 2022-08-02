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
	// 创建顶点输入布局
	HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	pImpl->m_pEffectHelper->CreateShaderFromFile("NormalMapVS", L"HLSL/NormalMap_VS.cso", device, "VS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf());
	HR(device->CreateInputLayout(VertexPosNormalTangentTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTangentTex::GetInputLayout()),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTangentTexLayout.GetAddressOf()));

	// 创建像素着色器
	pImpl->m_pEffectHelper->CreateShaderFromFile("BasicPS", L"HLSL/Basic_PS.cso", device, "PS", "ps_5_0");
	pImpl->m_pEffectHelper->CreateShaderFromFile("NormalMapPS", L"HLSL/NormalMap_PS.cso", device, "PS", "ps_5_0");

	// 创建通道
	EffectPassDesc passDesc;
	passDesc.nameVS = "BasicVS";
	passDesc.namePS = "BasicPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Basic", device, &passDesc));

	passDesc.nameVS = "NormalMapVS";
	passDesc.namePS = "NormalMapPS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("NormalMap", device, &passDesc));

	pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
	SetDebugObjectName(pImpl->m_pVertexPosNormalTangentTexLayout.Get(), "BasicEffect.VertexPosNormalTangentTexLayout");
#endif
	pImpl->m_pEffectHelper->SetDebugObjectName("BasicEffect");

	return true;
}

void BasicEffect::SetTextureCube(ID3D11ShaderResourceView* textureCube)
{
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_TexCube", textureCube);
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
	phongMat.reflect = material.Get<XMFLOAT4>("$ReflectColor");
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Material")->SetRaw(&phongMat);

	auto pStr = material.TryGet<std::string>("$Diffuse");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", pStr ? tm.GetTexture(*pStr) : nullptr);
	pStr = material.TryGet<std::string>("$Normal");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_NormalMap", pStr ? tm.GetTexture(*pStr) : nullptr);
}

MeshDataInput BasicEffect::GetInputData(const MeshData& meshData)
{
	MeshDataInput input;
	input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
	input.topology = pImpl->m_CurrTopology;

	if (pImpl->m_NormalmapEnabled)
	{
		input.pVertexBuffers =
		{
			meshData.m_pVertices.Get(),
			meshData.m_pNormals.Get(),
			meshData.m_pTangents.Get(),
			meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
		};
		input.strides = { 12,12,16,8 };
		input.offsets = { 0,0,0,0 };
	}
	else
	{
		input.pVertexBuffers =
		{
			meshData.m_pVertices.Get(),
			meshData.m_pNormals.Get(),
			meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get(),
			nullptr
		};
		input.strides = { 12,12,8,0 };
		input.offsets = { 0,0,0,0 };
	}

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
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogColor")->SetFloatVector(1, reinterpret_cast<const float*>(&fogColor));
}

void BasicEffect::SetFogRange(float fogRange)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_FogRange")->SetFloat(fogRange);
}

void BasicEffect::SetRenderDefault()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Basic");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_NormalmapEnabled = false;
}

void BasicEffect::SetRenderWithNormalMap()
{
	pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("NormalMap");
	pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTangentTexLayout;
	pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pImpl->m_NormalmapEnabled = true;
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