#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>
#include <DXTrace.h>
#include <Vertex.h>
#include "LightHelper.h"
using namespace DirectX;


static void GenerateGaussianWeight(float weight[], int kernelSize, float sigma) 
{
	float twoSigmaSq = 2.0f * sigma * sigma;
	int radius = kernelSize / 2;
	float sum = 0.0f;
	for (int i = -radius; i <= radius; ++i) 
	{
		float x = (float)i;
		weight[radius + i] = expf(-x * x / twoSigmaSq);
		sum += weight[radius + i];
	}

	// 标准化权值使得权值和为1.0
	for (int i = 0; i <= kernelSize; ++i) 
	{
		weight[i] /= sum;
	}
}

class PostProcessEffect::Impl 
{
public:
	Impl() {};
	~Impl() = default;
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	std::unique_ptr<EffectHelper> m_pEffectHelper;

	float m_Weights[32]{};
	int m_BlurRadius = 3;
	float m_BlurSigma = 1.0f;
};

namespace 
{
	static PostProcessEffect* g_pInstance = nullptr;
}


PostProcessEffect::PostProcessEffect()
{
	if (g_pInstance)
		throw std::exception("PostProcessEffect is a singleton!");
	g_pInstance = this;
	pImpl = std::make_unique<PostProcessEffect::Impl>();
}

PostProcessEffect::~PostProcessEffect()
{
}

PostProcessEffect::PostProcessEffect(PostProcessEffect&& moveForm) noexcept
{
	pImpl.swap(moveForm.pImpl);
}

PostProcessEffect& PostProcessEffect::operator=(PostProcessEffect&& moveForm) noexcept
{
	pImpl.swap(moveForm.pImpl);
	return *this;
}

PostProcessEffect& PostProcessEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("PostProcessEffect needs an instance!");
	return *g_pInstance;
}

bool PostProcessEffect::InitAll(ID3D11Device* device)
{
	if (!device)
		return false;
	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	// 创建计算着色器
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("BlurHorzCS", L"HLSL\\Blur_Horz_CS.cso", device));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("BlurVertCS", L"HLSL\\Blur_Vert_CS.cso", device));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("CompositeVS", L"HLSL\\Composite_VS.cso", device));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("CompositePS", L"HLSL\\Composite_PS.cso", device));
	HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SobelCS", L"HLSL\\Sobel_CS.cso", device));

	EffectPassDesc passDesc;
	passDesc.nameCS = "BlurHorzCS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("BlurHorz", device, &passDesc));
	passDesc.nameCS = "BlurVertCS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("BlurVert", device, &passDesc));
	passDesc.nameCS = "SobelCS";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Sobel", device, &passDesc));
	passDesc.nameVS = "CompositeVS";
	passDesc.namePS = "CompositePS";
	passDesc.nameCS = "";
	HR(pImpl->m_pEffectHelper->AddEffectPass("Composite", device, &passDesc));

	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamLinearWrap", RenderStates::SSLinearWrap.Get());
	pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamPointClamp", RenderStates::SSPointClamp.Get());

	pImpl->m_pEffectHelper->SetDebugObjectName("PostProcessEffect");

	return true;
}

void PostProcessEffect::RenderComposite(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* input1, 
	ID3D11ShaderResourceView* input2,
	ID3D11RenderTargetView* output,
	const D3D11_VIEWPORT& vp)
{
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("Composite");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Input", input1);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_EdgeInput", input2);
	pPass->Apply(deviceContext);

	deviceContext->OMSetRenderTargets(1, &output, nullptr);
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->Draw(3, 0);

	// 清空
	input1 = nullptr;
	deviceContext->PSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Input"), 1, &input1);
	deviceContext->PSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_EdgeInput"), 1, &input2);
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void PostProcessEffect::ComputeSobel(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* input, 
	ID3D11UnorderedAccessView* output, 
	uint32_t width, uint32_t height)
{
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("Sobel");
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Input", input);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_Output", output);
	pPass->Apply(deviceContext);
	pPass->Dispatch(deviceContext, width, height);

	input = nullptr;
	output = nullptr;
	deviceContext->CSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Input"), 1, &input);
	deviceContext->CSSetUnorderedAccessViews(pImpl->m_pEffectHelper->MapUnorderedAccessSlot("g_Output"), 1, &output, nullptr);
}

void PostProcessEffect::SetBlurKernelSize(int size)
{
	if (size % 2 == 0 || size > ARRAYSIZE(pImpl->m_Weights))
	{
		return;
	}
	pImpl->m_BlurRadius = size / 2;
	GenerateGaussianWeight(pImpl->m_Weights, size, pImpl->m_BlurSigma);
}

void PostProcessEffect::SetBlurSigma(float sigma)
{
	if (sigma < 0.0f) 
	{
		return;
	}
	pImpl->m_BlurSigma = sigma;
	GenerateGaussianWeight(pImpl->m_Weights, pImpl->m_BlurRadius * 2 + 1, pImpl->m_BlurSigma);
}

void PostProcessEffect::ComputeGaussianBlurX(
	ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView* input,
	ID3D11UnorderedAccessView* output,
	uint32_t width, uint32_t height)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("BlurHorz");
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Weights")->SetRaw(pImpl->m_Weights);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_BlurRadius")->SetSInt(pImpl->m_BlurRadius);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Input", input);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_Output", output);
	pPass->Apply(deviceContext);
	pPass->Dispatch(deviceContext, width, height);

	input = nullptr;
	output = nullptr;
	deviceContext->CSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Input"), 1, &input);
	deviceContext->CSSetUnorderedAccessViews(pImpl->m_pEffectHelper->MapUnorderedAccessSlot("g_Output"), 1, &output, nullptr);
}

void PostProcessEffect::ComputeGaussianBlurY(
	ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView* input,
	ID3D11UnorderedAccessView* output,
	uint32_t width, uint32_t height)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("BlurVert");
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Weights")->SetRaw(pImpl->m_Weights);
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_BlurRadius")->SetSInt(pImpl->m_BlurRadius);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Input", input);
	pImpl->m_pEffectHelper->SetUnorderedAccessByName("g_Output", output);
	pPass->Apply(deviceContext);
	pPass->Dispatch(deviceContext, width, height);

	input = nullptr;
	output = nullptr;
	deviceContext->CSSetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Input"), 1, &input);
	deviceContext->CSSetUnorderedAccessViews(pImpl->m_pEffectHelper->MapUnorderedAccessSlot("g_Output"), 1, &output, nullptr);
}

void PostProcessEffect::RenderScreenFade(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* input, 
	ID3D11RenderTargetView* output, 
	const D3D11_VIEWPORT& vp,
	float fadeAmount)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->OMSetRenderTargets(1, &output, nullptr);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Tex", input);
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("ScreenFade");
	pPass->PSGetParamByName("fadeAmount")->SetFloat(fadeAmount);
	pPass->Apply(deviceContext);
	deviceContext->Draw(3, 0);

	// 清空
	output = nullptr;
	input = nullptr;
	deviceContext->OMSetRenderTargets(0, &output, nullptr);
	deviceContext->PSGetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Tex"), 1, &input);
}

void PostProcessEffect::SetVisibleRange(float range)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_VisibleRange")->SetFloat(range);
}

void PostProcessEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EyePosW")->SetFloatVector(3, reinterpret_cast<const float*>(&eyePos));
}

void PostProcessEffect::SetMinimapRect(const DirectX::XMFLOAT4& rect)
{
	pImpl->m_pEffectHelper->GetConstantBufferVariable("g_RectW")->SetFloatVector(4, reinterpret_cast<const float*>(&rect));
}

void PostProcessEffect::RenderMinimap(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* input,
	ID3D11RenderTargetView* output,
	const D3D11_VIEWPORT& vp)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->OMSetRenderTargets(1, &output, nullptr);
	pImpl->m_pEffectHelper->SetShaderResourceByName("g_Tex", input);
	auto pPass = pImpl->m_pEffectHelper->GetEffectPass("Minimap");
	pPass->Apply(deviceContext);
	deviceContext->Draw(3, 0);

	// 清空
	output = nullptr;
	input = nullptr;
	deviceContext->OMSetRenderTargets(0, &output, nullptr);
	deviceContext->PSGetShaderResources(pImpl->m_pEffectHelper->MapShaderResourceSlot("g_Tex"),1, &input);
}
