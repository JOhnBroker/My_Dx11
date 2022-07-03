#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"
using namespace DirectX;

class BasicEffect::Impl :public AlignedType<BasicEffect::Impl>
{
public:
	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		Material material;
	};

	struct CBDrawingStates
	{
		int isReflection;
		int isShadow;
		DirectX::XMINT2 pad;
	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT3 eyePos;
		float pad;
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely
	{
		DirectX::XMMATRIX reflection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX refShadow;
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
	};

public:
	Impl() : m_IsDirty() {}
	~Impl() = default;

public:
	// 需要16字节对齐的优先放在前面
	CBufferObject<0, CBChangesEveryDrawing> m_CBDrawing;		// 每次对象绘制的常量缓冲区
	CBufferObject<1, CBDrawingStates>       m_CBStates;		    // 每次绘制状态变更的常量缓冲区
	CBufferObject<2, CBChangesEveryFrame>   m_CBFrame;		    // 每帧绘制的常量缓冲区
	CBufferObject<3, CBChangesOnResize>     m_CBOnResize;		// 每次窗口大小变更的常量缓冲区
	CBufferObject<4, CBChangesRarely>		m_CBRarely;		    // 几乎不会变更的常量缓冲区
	BOOL m_IsDirty;												// 是否有值变更
	std::vector<CBufferBase*> m_pCBuffers;					    // 统一管理上面所有的常量缓冲区


	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShader3D;				// 用于3D的像素着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;				// 用于2D的顶点着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShader2D;				// 用于2D的像素着色器

	ComPtr<ID3D11InputLayout>  m_pVertexLayout2D;				// 用于2D的顶点输入布局
	ComPtr<ID3D11InputLayout>  m_pVertexLayout3D;				// 用于3D的顶点输入布局

	ComPtr<ID3D11ShaderResourceView> m_pTexture;				// 用于绘制的纹理
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
	{
		throw std::exception("BasicEffect is a singleton!");
	}
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
	if(!g_pInstance)
	{
		throw std::exception("BasicEffect needs an instance!");
	}
	return *g_pInstance;
}

bool BasicEffect::InitAll(ID3D11Device* device)
{
	return false;
}

void BasicEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext)
{
}

 void BasicEffect::SetRenderAlphaBlend(ID3D11DeviceContext* deviceContext)
{
}

 void BasicEffect::SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext)
{
}

 void BasicEffect::SetWriteStencilOnly(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
}

 void BasicEffect::SetRenderDefaultWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
}

 void BasicEffect::SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
}

 void BasicEffect::Set2DRenderDefault(ID3D11DeviceContext* deviceContext)
{
}

 void BasicEffect::Set2DRenderAlphaBlend(ID3D11DeviceContext* deviceContext)
{
}

 void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	 auto& cBuffer = pImpl->m_CBDrawing;
	 cBuffer.data.world = XMMatrixTranspose(W);
	 cBuffer.data.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void XM_CALLCONV BasicEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
	 auto& cBuffer = pImpl->m_CBFrame;
	 cBuffer.data.view = XMMatrixTranspose(V);
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void XM_CALLCONV BasicEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
	 auto& cBuffer = pImpl->m_CBOnResize;
	 cBuffer.data.proj = XMMatrixTranspose(P);
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void XM_CALLCONV BasicEffect::SetReflectionMatrix(DirectX::FXMMATRIX R)
{
	 auto& cBuffer = pImpl->m_CBRarely;
	 cBuffer.data.reflection = XMMatrixTranspose(R);
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void XM_CALLCONV BasicEffect::SetShadowMatrix(DirectX::FXMMATRIX S)
{
	 auto& cBuffer = pImpl->m_CBRarely;
	 cBuffer.data.shadow = XMMatrixTranspose(S);
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void XM_CALLCONV BasicEffect::SetRefShadowMatrix(DirectX::FXMMATRIX RefS)
{
	 auto& cBuffer = pImpl->m_CBRarely;
	 cBuffer.data.refShadow = XMMatrixTranspose(RefS);
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void BasicEffect::SetDirLight(size_t pos, const DirectionalLight& dirLight)
{
}

 void BasicEffect::SetPointLight(size_t pos, const PointLight& dirLight)
{
}

 void BasicEffect::SetSpotLight(size_t pos, const SpotLight& dirLight)
{
}

 void BasicEffect::SetMaterial(const Material& material)
{
}

 void BasicEffect::SetTexture(ID3D11ShaderResourceView* textuer)
{
}

 void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
}

 void BasicEffect::SetReflectionState(bool isOn)
{
	 auto& cBuffer = pImpl->m_CBStates;
	 cBuffer.data.isReflection = isOn;
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}

 void BasicEffect::SetShadowState(bool isOn)
{
	 auto& cBuffer = pImpl->m_CBStates;
	 cBuffer.data.isShadow = isOn;
	 pImpl->m_IsDirty = cBuffer.isDirty = true;
}
