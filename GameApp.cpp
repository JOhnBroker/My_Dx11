#include "GameApp.h"
#include "XUtil.h"
#include "DXTrace.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: D3DApp(hInstance, windowName, initWidth, initHeight),
	m_pEffectHelper(std::make_unique<EffectHelper>())
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	//m_TextureManager.Init(m_pd3dDevice.Get());
	//m_ModelManager.Init(m_pd3dDevice.Get());
	//
	//// 务必先初始化所有渲染状态，以供下面的特效使用
	//RenderStates::InitAll(m_pd3dDevice.Get());
	//
	//if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;
	//
	//if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;
	//
	//if (!m_ShadowEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;
	//
	//if (!m_SSAOEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();

	//m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
	//m_pLitTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	//m_pDebugAOTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth / 2, m_ClientHeight / 2, DXGI_FORMAT_R8G8B8A8_UNORM);
	//m_SSAOManager.OnResize(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
	//
	//m_pDepthTexture->SetDebugObjectName("DepthTexture");
	//m_pLitTexture->SetDebugObjectName("LitTexture");
	//m_pDebugAOTexture->SetDebugObjectName("DebugAOTexture");
	//
	//if (m_pCamera != nullptr)
	//{
	//	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	//	m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	//	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	//	m_SSAOEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	//}
}

void GameApp::UpdateScene(float dt)
{
	//m_CameraController.Update(dt);
	//
	//if (ImGui::Begin("SSAO"))
	//{
	//	ImGui::Checkbox("Animate Light", &m_UpdateLight);
	//	ImGui::Checkbox("Enable Normal map", &m_EnableNormalMap);
	//	ImGui::Separator();
	//	if (ImGui::Checkbox("Enable SSAO", &m_EnableSSAO))
	//	{
	//		if (!m_EnableSSAO)
	//		{
	//			m_EnableDebug = false;
	//		}
	//		m_BasicEffect.SetSSAOEnabled(m_EnableSSAO);
	//	}
	//	if (m_EnableSSAO)
	//	{
	//		ImGui::SliderFloat("Epsilon", &m_SSAOManager.m_SurfaceEpsilon, 0.0f, 0.1f, "%.2f");
	//		static float range = m_SSAOManager.m_OcclusionFadeEnd - m_SSAOManager.m_OcclusionFadeStart;
	//		ImGui::SliderFloat("Fade Start", &m_SSAOManager.m_OcclusionFadeStart, 0.0f, 2.0f, "%.2f");
	//		if (ImGui::SliderFloat("Fade Range", &range, 0.0f, 3.0f, "%.2f"))
	//		{
	//			m_SSAOManager.m_OcclusionFadeEnd = m_SSAOManager.m_OcclusionFadeStart + range;
	//		}
	//		ImGui::SliderFloat("Sample Radius", &m_SSAOManager.m_OcclusionRadius, 0.0f, 2.0f, "%.1f");
	//		ImGui::SliderInt("Sample Count", reinterpret_cast<int*>(&m_SSAOManager.m_SampleCount), 1, 14);
	//		ImGui::Checkbox("Debug SSAO", &m_EnableDebug);
	//	}
	//	ImGui::Separator();
	//}
	//ImGui::End();
	//
	//m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
	//m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//m_SSAOEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//
	//// 更新光照
	//static float theta = 0;
	//if (m_UpdateLight)
	//{
	//	theta += dt * XM_2PI / 40.0f;
	//}
	//for (int i = 0; i < 3; ++i)
	//{
	//	XMVECTOR dirVec = XMLoadFloat3(&m_OriginalLightDirs[i]);
	//	dirVec = XMVector3Transform(dirVec, XMMatrixRotationY(theta));
	//	XMStoreFloat3(&m_DirLights[i].direction, dirVec);
	//	m_BasicEffect.SetDirLight(i, m_DirLights[i]);
	//}
	//
	//// 投影区域为正方体，以原点为中心，以方向光为 +Z 朝向
	//XMVECTOR dirVec = XMLoadFloat3(&m_DirLights[0].direction);
	//XMMATRIX LightView = XMMatrixLookAtLH(dirVec * 20.0f * (-2.0f), g_XMZero, g_XMIdentityR1);
	//m_ShadowEffect.SetViewMatrix(LightView);
	//
	//// 将NDC空间 [-1, +1]^2 变换到纹理坐标空间 [0, 1]^2
	//static XMMATRIX T(
	//	0.5f, 0.0f, 0.0f, 0.0f,
	//	0.0f, -0.5f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	0.5f, 0.5f, 0.0f, 1.0f);
	//
	//// ShadowTransform = V * P * T
	//m_BasicEffect.SetShadowTransformMatrix(LightView * XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f) * T);
	if (ImGui::Begin("Tessellation"))
	{
		static int curr_item = 0;
		static const char* modes[] = {
			"Triangle",
			"Quad",
			"BezierCurve",
			"BezierSurface"
		};
		if (ImGui::Combo("Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			m_TessellationMode = static_cast<TessellationMode>(curr_item);
		}
	}

	switch (m_TessellationMode)
	{
	case GameApp::TessellationMode::Triangle:
		UpdateTriangle();
		break;
	case GameApp::TessellationMode::Quad:
		UpdateQuad();
		break;
	case GameApp::TessellationMode::BezierCurve:
		UpdateBezierCurve();
		break;
	case GameApp::TessellationMode::BezierSurface:
		UpdateBezierSurface();
		break;
	}

	ImGui::End();
	ImGui::Render();
}

void GameApp::DrawScene()
{
	// 创建后备缓冲区的渲染目标视图
	if (m_FrameCount < m_BackBufferCount)
	{
		ComPtr<ID3D11Texture2D> pBackBuffer;
		m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
		CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
	}

	//if (m_EnableSSAO)
	//{
	//	RenderSSAO();
	//}
	//RenderShadow();
	//RenderForward();
	//RenderSkybox();
	//
	//if (m_EnableDebug)
	//{
	//	if (ImGui::Begin("SSAO Buffer", &m_EnableDebug))
	//	{
	//		CD3D11_VIEWPORT vp(0.0f, 0.0f, (float)m_pDebugAOTexture->GetWidth(), (float)m_pDebugAOTexture->GetHeight());
	//		m_SSAOEffect.RenderAmbientOcclusionToTexture(
	//			m_pd3dImmediateContext.Get(),
	//			m_SSAOManager.GetAmbientOcclusionTexture(),
	//			m_pDebugAOTexture->GetRenderTarget(),
	//			vp);
	//		ImVec2 winSize = ImGui::GetWindowSize();
	//		float smaller = (std::min)((winSize.x - 20) / AspectRatio(), winSize.y - 36);
	//		ImGui::Image(m_pDebugAOTexture->GetShaderResource(), ImVec2(smaller * AspectRatio(), smaller));
	//	}
	//	ImGui::End();
	//}
	//ImGui::Render();
	//
	//ID3D11RenderTargetView* pRTVs[]{ GetBackBufferRTV() };
	//m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);

	m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), Colors::Black);
	ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	switch (m_TessellationMode)
	{
	case GameApp::TessellationMode::Triangle:
		DrawTriangle();
		break;
	case GameApp::TessellationMode::Quad:
		DrawQuad();
		break;
	case GameApp::TessellationMode::BezierCurve:
		DrawBezierCurve();
		break;
	case GameApp::TessellationMode::BezierSurface:
		DrawBezierSurface();
		break;
	}

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

}

void GameApp::UpdateTriangle()
{
	ImGui::SliderFloat3("TriEdgeTess", m_TriEdgeTess, 1.0f, 10.0f, "%.1f");
	ImGui::SliderFloat("TriInsideTess", &m_TriInsideTess, 1.0f, 10.0f, "%.1f");

	m_pEffectHelper->GetConstantBufferVariable("g_TriEdgeTess")->SetFloatVector(3, m_TriEdgeTess);
	m_pEffectHelper->GetConstantBufferVariable("g_TriInsideTess")->SetFloat(m_TriInsideTess);
}

void GameApp::UpdateQuad()
{
	static int part_item = 0;
	static const char* part_modes[] =
	{
		"Integer",
		"Odd",
		"Even"
	};
	if (ImGui::Combo("Partition Mode", &part_item, part_modes, ARRAYSIZE(part_modes)))
	{
		m_PartitionMode = static_cast<PartitionMode>(part_item);
	}
	ImGui::SliderFloat4("QuadEdgeTess", m_QuadEdgeTess, 1.0f, 10.0f, "%.1f");
	ImGui::SliderFloat2("QuadInsideTess", m_QuadInsideTess, 1.0f, 10.0f, "%.1f");

	m_pEffectHelper->GetConstantBufferVariable("g_QuadEdgeTess")->SetFloatVector(4, m_QuadEdgeTess);
	m_pEffectHelper->GetConstantBufferVariable("g_QuadInsideTess")->SetFloatVector(4, m_QuadInsideTess);

}

void GameApp::UpdateBezierCurve()
{
	bool c1_continuity = false;
	ImGui::SliderFloat("IsolineEdgeTess", &m_IsolineEdgeTess[1], 1.0f, 64.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
	if (ImGui::Button("C1 Continuity"))
	{
		c1_continuity = true;
	}
	ImGuiIO& io = ImGui::GetIO();

	XMFLOAT3 worldPos = XMFLOAT3(
		(2.0f * io.MousePos.x / m_ClientWidth - 1.0f) * AspectRatio(),
		-2.0f * io.MousePos.y / m_ClientHeight + 1.0f,
		0.0f);
	float dy = 12.0f / m_ClientHeight;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		for (int i = 0; i < 10; ++i)
		{
			if (fabs(worldPos.x - m_BezPoints[i].x) <= dy && fabs(worldPos.y - m_BezPoints[i].y) <= dy)
			{
				m_ChosenBezPoint = i;
				break;
			}
		}
	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		m_ChosenBezPoint = -1;
	}
	else if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_ChosenBezPoint >= 0 &&
		io.MousePos.x > 5 && io.MousePos.x < m_ClientWidth - 5 &&
		io.MousePos.y >5 && io.MousePos.y < m_ClientHeight - 5)
	{
		m_BezPoints[m_ChosenBezPoint] = worldPos;
	}
	if (c1_continuity)
	{
		XMVECTOR posVec2 = XMLoadFloat3(&m_BezPoints[2]);
		XMVECTOR posVec3 = XMLoadFloat3(&m_BezPoints[3]);
		XMVECTOR posVec5 = XMLoadFloat3(&m_BezPoints[5]);
		XMVECTOR posVec6 = XMLoadFloat3(&m_BezPoints[6]);
		XMStoreFloat3(m_BezPoints + 4, posVec2 + 2 * (posVec3 - posVec2));
		XMStoreFloat3(m_BezPoints + 7, posVec5 + 2 * (posVec6 - posVec5));
	}

	XMMATRIX WVP = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), g_XMZero, g_XMIdentityR1) *
		XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 0.1f, 1000.0f);
	WVP = XMMatrixTranspose(WVP);
	m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&WVP);
	m_pEffectHelper->GetConstantBufferVariable("g_IsolineEdgeTess")->SetFloatVector(2, m_IsolineEdgeTess);
	m_pEffectHelper->GetConstantBufferVariable("g_InvScreenHeight")->SetFloat(1.0f / m_ClientHeight);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pBezCurveVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	for (size_t i = 0; i < 3; ++i)
	{
		memcpy_s(reinterpret_cast<XMFLOAT3*>(mappedData.pData) + i * 4, sizeof(XMFLOAT3) * 4, m_BezPoints + i * 3, sizeof(XMFLOAT3) * 4);
	}
	m_pd3dImmediateContext->Unmap(m_pBezCurveVB.Get(), 0);
}

void GameApp::UpdateBezierSurface()
{
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_Theta = XMScalarModAngle(m_Theta + io.MouseDelta.x * 0.01f);
		m_Phi += -io.MouseDelta.y * 0.01f;
	}
	m_Radius -= io.MouseWheel;

	// 限制Phi
	m_Phi = std::clamp(m_Phi, XM_PI / 18, 1.0f - XM_PI / 18);
	// 限制半径
	m_Radius = std::clamp(m_Radius, 1.0f, 100.0f);

	XMVECTOR posVec = XMVectorSet(
		m_Radius * sinf(m_Phi) * cosf(m_Theta),
		m_Radius * cosf(m_Phi),
		m_Radius * sinf(m_Phi) * sinf(m_Theta),
		0.0f);

	XMMATRIX WVP = XMMatrixLookAtLH(posVec, g_XMZero, g_XMIdentityR1) *
		XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 0.1f, 1000.0f);
	WVP = XMMatrixTranspose(WVP);
	m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&WVP);
	m_pEffectHelper->GetConstantBufferVariable("g_QuadEdgeTess")->SetFloatVector(4, m_QuadPatchEdgeTess);
	m_pEffectHelper->GetConstantBufferVariable("g_QuadInsideTess")->SetFloatVector(2, m_QuadPatchInsideTess);
}

void GameApp::DrawTriangle()
{
	UINT strides[1] = { sizeof(XMFLOAT3) };
	UINT offsets[1] = { 0 };
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pTriangleVB.GetAddressOf(), strides, offsets);
	m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::White);
	m_pEffectHelper->GetEffectPass("Tessellation_Triangle")->Apply(m_pd3dImmediateContext.Get());

	m_pd3dImmediateContext->Draw(3, 0);
}

void GameApp::DrawQuad()
{
	UINT strides[1] = { sizeof(XMFLOAT3) };
	UINT offsets[1] = { 0 };
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pQuadVB.GetAddressOf(), strides, offsets);
	m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::White);
	switch (m_PartitionMode)
	{
	case GameApp::PartitionMode::Integer:
		m_pEffectHelper->GetEffectPass("Tessellation_Quad_Integer")->Apply(m_pd3dImmediateContext.Get());
		break;
	case GameApp::PartitionMode::Odd:
		m_pEffectHelper->GetEffectPass("Tessellation_Quad_Odd")->Apply(m_pd3dImmediateContext.Get());
		break;
	case GameApp::PartitionMode::Even:
		m_pEffectHelper->GetEffectPass("Tessellation_Quad_Even")->Apply(m_pd3dImmediateContext.Get());
		break;
	}

	m_pd3dImmediateContext->Draw(4, 0);
}

void GameApp::DrawBezierCurve()
{
	UINT strides[1] = { sizeof(XMFLOAT3) };
	UINT offsets[1] = { 0 };
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pBezCurveVB.GetAddressOf(), strides, offsets);
	m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());

	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::Red);
	m_pEffectHelper->GetEffectPass("Tessellation_Point2Square")->Apply(m_pd3dImmediateContext.Get());
	m_pd3dImmediateContext->Draw(12, 0);

	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::White);
	m_pEffectHelper->GetEffectPass("Tessellation_BezierCurve")->Apply(m_pd3dImmediateContext.Get());
	m_pd3dImmediateContext->Draw(12, 0);

	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::Red);
	m_pEffectHelper->GetEffectPass("Tessellation_NoTess")->Apply(m_pd3dImmediateContext.Get());
	m_pd3dImmediateContext->Draw(12, 0);
}

void GameApp::DrawBezierSurface()
{
	UINT strides[1] = { sizeof(XMFLOAT3) };
	UINT offsets[1] = { 0 };
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pBezSurfaceVB.GetAddressOf(), strides, offsets);
	m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::White);
	m_pEffectHelper->GetEffectPass("Tessellation_BezierSurface")->Apply(m_pd3dImmediateContext.Get());

	m_pd3dImmediateContext->Draw(16, 0);
}

void GameApp::RenderSSAO()
{
	//// Pass1 绘制场景 生成法线深度图
	//m_SSAOManager.Begin(m_pd3dImmediateContext.Get(), m_pDepthTexture->GetDepthStencil(), m_pCamera->GetViewPort());
	//{
	//	m_SSAOEffect.SetRenderNormalDepthMap(m_pd3dImmediateContext.Get());
	//	DrawScene<SSAOEffect>(m_SSAOEffect);
	//}
	//m_SSAOManager.End(m_pd3dImmediateContext.Get());
	//// Pass2 生产AO
	//m_SSAOManager.RenderToSSAOTexture(m_pd3dImmediateContext.Get(), m_SSAOEffect, *m_pCamera);
	//
	//// Pass3 混合
	//m_SSAOManager.BlurAmbientMap(m_pd3dImmediateContext.Get(), m_SSAOEffect);
}

void GameApp::RenderShadow()
{
	//CD3D11_VIEWPORT shadowViewport(0.0f, 0.0f, (float)m_pShadowMapTexture->GetWidth(), (float)m_pShadowMapTexture->GetHeight());
	//m_pd3dImmediateContext->ClearDepthStencilView(m_pShadowMapTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, m_pShadowMapTexture->GetDepthStencil());
	//m_pd3dImmediateContext->RSSetViewports(1, &shadowViewport);
	//
	//m_ShadowEffect.SetRenderDepthOnly();
	//DrawScene<ShadowEffect>(m_ShadowEffect);
}

void GameApp::RenderForward()
{
	//float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	//ID3D11RenderTargetView* pRTVs[]{ m_pLitTexture->GetRenderTarget() };
	//m_pd3dImmediateContext->ClearRenderTargetView(pRTVs[0], black);
	//if (!m_EnableSSAO)
	//{
	//	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//}
	//m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	//D3D11_VIEWPORT vp = m_pCamera->GetViewPort();
	//m_pd3dImmediateContext->RSSetViewports(1, &vp);
	//
	//m_BasicEffect.SetTextureShadowMap(m_pShadowMapTexture->GetShaderResource());
	//m_BasicEffect.SetTextureAmbientOcclusion(m_EnableSSAO ? m_SSAOManager.GetAmbientOcclusionTexture() : nullptr);
	//
	//if (m_EnableNormalMap)
	//{
	//	m_BasicEffect.SetRenderWithNormalMap();
	//}
	//else
	//{
	//	m_BasicEffect.SetRenderDefault();
	//}
	//DrawScene<BasicEffect>(m_BasicEffect, [](BasicEffect& effect, ID3D11DeviceContext* deviceContext)
	//{
	//	effect.SetRenderDefault();
	//});
	//
	//m_BasicEffect.SetTextureShadowMap(nullptr);
	//m_BasicEffect.SetTextureAmbientOcclusion(nullptr);
	//m_BasicEffect.Apply(m_pd3dImmediateContext.Get());
}
void GameApp::RenderSkybox()
{
	//D3D11_VIEWPORT skyboxViewport = m_pCamera->GetViewPort();
	//skyboxViewport.MinDepth = 1.0f;
	//skyboxViewport.MaxDepth = 1.0f;
	//m_pd3dImmediateContext->RSSetViewports(1, &skyboxViewport);
	//
	//m_SkyboxEffect.SetRenderDefault();
	//m_SkyboxEffect.SetDepthTexture(m_pDepthTexture->GetShaderResource());
	//m_SkyboxEffect.SetLitTexture(m_pLitTexture->GetShaderResource());
	//
	//// 由于全屏绘制，不需要用到深度缓冲区，也就不需要清空后备缓冲区了
	//ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
	//m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	//m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
	//
	//m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	//m_SkyboxEffect.SetDepthTexture(nullptr);
	//m_SkyboxEffect.SetLitTexture(nullptr);
	//m_SkyboxEffect.Apply(m_pd3dImmediateContext.Get());
}

void GameApp::DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
{
	/*
		float black[4] = { 0.0f,0.0f,0.0f,1.0f };
		m_pd3dImmediateContext->ClearRenderTargetView(pRTV, black);
		m_pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);

		BoundingFrustum frustum;
		BoundingFrustum::CreateFromMatrix(frustum, camera.GetProjMatrixXM());
		frustum.Transform(frustum, camera.GetLocalToWorldMatrixXM());
		D3D11_VIEWPORT viewport = camera.GetViewPort();
		m_pd3dImmediateContext->RSSetViewports(1, &viewport);

		m_BasicEffect.SetViewMatrix(camera.GetViewMatrixXM());
		m_BasicEffect.SetProjMatrix(camera.GetProjMatrixXM());
		m_BasicEffect.SetEyePos(camera.GetPosition());
		m_BasicEffect.SetRenderDefault();

		if (drawCenterSphere)
		{
			m_BasicEffect.SetReflectionEnabled(true);
			m_BasicEffect.SetRefractionEnabled(false);
			m_BasicEffect.SetTextureCube(m_pDynamicTextureCube->GetShaderResource());
			m_CenterSphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
			m_BasicEffect.SetTextureCube(nullptr);
		}

		m_BasicEffect.SetReflectionEnabled(false);
		m_BasicEffect.SetRefractionEnabled(false);

		if (m_EnableNormalMap)
		{
			m_BasicEffect.SetRenderWithNormalMap();
		}
		else
		{
			m_BasicEffect.SetRenderDefault();
		}
		m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

		// 绘制五个圆柱
		for (auto& cylinder : m_Cylinders)
		{
			cylinder.FrustumCulling(frustum);
			cylinder.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		}

		m_BasicEffect.SetRenderDefault();
		// 绘制五个圆球
		for (auto& sphere : m_Spheres)
		{
			sphere.FrustumCulling(frustum);
			sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		}

		m_SkyBoxEffect.SetViewMatrix(camera.GetViewMatrixXM());
		m_SkyBoxEffect.SetProjMatrix(camera.GetProjMatrixXM());
		m_SkyBoxEffect.SetRenderDefault();
		m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyBoxEffect);
	*/
}


bool GameApp::InitResource()
{
	/*

	// ******************
	// 初始化摄像机
	//

	auto camera = std::make_shared<FirstPersonCamera>();
	m_pCamera = camera;

	//camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	//camera->SetTarget(XMFLOAT3(0.0f, 2.5f, 0.0f));
	//camera->SetDistance(20.0f);
	//camera->SetDistanceMinMax(10.0f, 90.0f);
	//camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	//camera->SetRotationX(XM_PIDIV4);

	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_CameraController.InitCamera(m_pCamera.get());

	// 初始化阴影贴图和特效
	m_pShadowMapTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), 2048, 2048);

	m_pShadowMapTexture->SetDebugObjectName("ShadowMapTexture");

	m_BasicEffect.SetSSAOEnabled(m_EnableSSAO);
	m_BasicEffect.SetDepthBias(0.005f);
	m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());		//忘记设置投影矩阵了

	m_SSAOEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_SSAOEffect.SetProjMatrix(camera->GetProjMatrixXM());

	m_ShadowEffect.SetProjMatrix(XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f));

	m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());

	m_SSAOManager.InitResource(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);

	// 初始化对象
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreatePlane(XMFLOAT2(20.0f, 30.0f), XMFLOAT2(6.0f, 9.0f)));
		pModel->SetDebugObjectName("Ground");
		m_TextureManager.CreateFromFile("Texture\\floor.dds", false, true);
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\floor.dds");
		m_TextureManager.CreateFromFile("Texture\\floor_nmap.dds");
		pModel->materials[0].Set<std::string>("$Normal", "Texture\\floor_nmap.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		m_Ground.SetModel(pModel);
		m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);
	}
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Cylinder", Geometry::CreateCylinder(0.5f, 3.0f));
		pModel->SetDebugObjectName("Cylinder");
		m_TextureManager.CreateFromFile("Texture\\bricks.dds", false, true);
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\bricks.dds");
		m_TextureManager.CreateFromFile("Texture\\bricks_nmap.dds");
		pModel->materials[0].Set<std::string>("$Normal", "Texture\\bricks_nmap.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		for (size_t i = 0; i < 10; ++i)
		{
			m_Cylinders[i].SetModel(pModel);
			m_Cylinders[i].GetTransform().SetPosition(-6.0f + 12.0f * (i / 5), -1.5f, -10.0f + (i % 5) * 5.0f);
		}
	}
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Sphere", Geometry::CreateSphere(0.5f));
		pModel->SetDebugObjectName("Sphere");
		m_TextureManager.CreateFromFile("Texture\\stone.dds", false, true);
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\stone.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		for (size_t i = 0; i < 10; ++i)
		{
			m_Spheres[i].SetModel(pModel);
			m_Spheres[i].GetTransform().SetPosition(-6.0f + 12.0f * (i / 5), 0.5f, -10.0f + (i % 5) * 5.0f);
		}
	}
	{
		Model* pModel = m_ModelManager.CreateFromFile("Model\\house.obj");
		pModel->SetDebugObjectName("House");
		m_House.SetModel(pModel);

		XMMATRIX S = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		BoundingBox houseBox = m_House.GetBoundingBox();
		houseBox.Transform(houseBox, S);

		Transform& houseTransform = m_House.GetTransform();
		houseTransform.SetScale(0.01f, 0.01f, 0.01f);
		houseTransform.SetPosition(0.0f, -(houseBox.Center.y - houseBox.Extents.y + 3.0f), 0.0f);
	}
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
		pModel->SetDebugObjectName("Skybox");
		m_Skybox.SetModel(pModel);
		m_TextureManager.CreateFromFile("Texture\\desertcube1024.dds", false, true);
		pModel->materials[0].Set<std::string>("$Skybox", "Texture\\desertcube1024.dds");
	}

	// ******************
	// 初始化不会变化的值
	//

	// 环境光
	m_DirLights[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[0].diffuse = XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
	m_DirLights[0].specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	m_DirLights[0].direction = XMFLOAT3(5.0f / sqrtf(50.0f), -5.0f / sqrtf(50.0f), 0.0f);

	m_DirLights[1].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[1].diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	m_DirLights[1].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_DirLights[2].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	for (int i = 0; i < 3; ++i)
	{
		m_OriginalLightDirs[i] = m_DirLights[i].direction;
		m_BasicEffect.SetDirLight(i, m_DirLights[i]);
	}
	*/

	XMFLOAT3 triVertices[3] = {
		XMFLOAT3(-0.6f,-0.8f,0.0f),
		XMFLOAT3(0.0f,0.8f,0.0f),
		XMFLOAT3(0.6f,-0.8f,0.0f)
	};
	CD3D11_BUFFER_DESC bufferDesc(sizeof(triVertices), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA initData{ triVertices };
	HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, m_pTriangleVB.GetAddressOf()));

	XMFLOAT3 quadVertices[4] = {
		XMFLOAT3(-0.4f, 0.72f, 0.0f),
		XMFLOAT3(0.4f, 0.72f, 0.0f),
		XMFLOAT3(-0.4f, -0.72f, 0.0f),
		XMFLOAT3(0.4f, -0.72f, 0.0f)
	};
	bufferDesc.ByteWidth = sizeof quadVertices;
	initData.pSysMem = quadVertices;
	HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, m_pQuadVB.GetAddressOf()));

	XMFLOAT3 surfaceVertices[16] = {
		// 行 0
		XMFLOAT3(-10.0f, -10.0f, +15.0f),
		XMFLOAT3(-5.0f,  0.0f, +15.0f),
		XMFLOAT3(+5.0f,  0.0f, +15.0f),
		XMFLOAT3(+10.0f, 0.0f, +15.0f),

		// 行 1
		XMFLOAT3(-15.0f, 0.0f, +5.0f),
		XMFLOAT3(-5.0f,  0.0f, +5.0f),
		XMFLOAT3(+5.0f,  20.0f, +5.0f),
		XMFLOAT3(+15.0f, 0.0f, +5.0f),

		// 行 2
		XMFLOAT3(-15.0f, 0.0f, -5.0f),
		XMFLOAT3(-5.0f,  0.0f, -5.0f),
		XMFLOAT3(+5.0f,  0.0f, -5.0f),
		XMFLOAT3(+15.0f, 0.0f, -5.0f),

		// 行 3
		XMFLOAT3(-10.0f, 10.0f, -15.0f),
		XMFLOAT3(-5.0f,  0.0f, -15.0f),
		XMFLOAT3(+5.0f,  0.0f, -15.0f),
		XMFLOAT3(+25.0f, 10.0f, -15.0f)
	};
	bufferDesc.ByteWidth = sizeof surfaceVertices;
	initData.pSysMem = surfaceVertices;
	HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, m_pBezSurfaceVB.GetAddressOf()));

	m_BezPoints[0] = XMFLOAT3(-0.8f, -0.2f, 0.0f);
	m_BezPoints[1] = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	m_BezPoints[2] = XMFLOAT3(-0.5f, 0.6f, 0.0f);
	m_BezPoints[3] = XMFLOAT3(-0.35f, 0.6f, 0.0f);
	m_BezPoints[4] = XMFLOAT3(-0.2f, 0.6f, 0.0f);
	m_BezPoints[5] = XMFLOAT3(0.1f, 0.0f, 0.0f);
	m_BezPoints[6] = XMFLOAT3(0.1f, -0.3f, 0.0f);
	m_BezPoints[7] = XMFLOAT3(0.4f, -0.3f, 0.0f);
	m_BezPoints[8] = XMFLOAT3(0.6f, 0.6f, 0.0f);
	m_BezPoints[9] = XMFLOAT3(0.8f, 0.4f, 0.0f);

	// 动态更新
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * 12;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, m_pBezCurveVB.GetAddressOf()));

	CD3D11_RASTERIZER_DESC rasterizerDesc(CD3D11_DEFAULT{});
	// 线框模式
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireFrame.GetAddressOf()));

	ComPtr<ID3DBlob> blob;
	D3D11_INPUT_ELEMENT_DESC inputElemDesc[1] = {
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_pEffectHelper->SetBinaryCacheDirectory(L"HLSL\\Cache");

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_VS", L"HLSL\\Tessellation_VS.hlsl",
		m_pd3dDevice.Get(), "VS", "vs_5_0", nullptr, blob.GetAddressOf()));
	HR(m_pd3dDevice->CreateInputLayout(inputElemDesc, 1, blob->GetBufferPointer(), blob->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Transform_VS", L"HLSL\\Tessellation_Transform_VS.hlsl",
		m_pd3dDevice.Get(), "VS", "vs_5_0"));

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Isoline_HS", L"HLSL\\Tessellation_Isoline_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Quad_Integer_HS", L"HLSL\\Tessellation_Quad_Integer_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Quad_Odd_HS", L"HLSL\\Tessellation_Quad_Odd_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Quad_Even_HS", L"HLSL\\Tessellation_Quad_Even_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Triangle_HS", L"HLSL\\Tessellation_Triangle_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_BezierSurface_HS", L"HLSL\\Tessellation_BezierSurface_HS.hlsl",
		m_pd3dDevice.Get(), "HS", "hs_5_0"));

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_BezierCurve_DS", L"HLSL\\Tessellation_BezierCurve_DS.hlsl",
		m_pd3dDevice.Get(), "DS", "ds_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_BezierSurface_DS", L"HLSL\\Tessellation_BezierSurface_DS.hlsl",
		m_pd3dDevice.Get(), "DS", "ds_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Quad_DS", L"HLSL\\Tessellation_Quad_DS.hlsl",
		m_pd3dDevice.Get(), "DS", "ds_5_0"));
	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Triangle_DS", L"HLSL\\Tessellation_Triangle_DS.hlsl",
		m_pd3dDevice.Get(), "DS", "ds_5_0"));

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_Point2Square_GS", L"HLSL\\Tessellation_Point2Square_GS.hlsl",
		m_pd3dDevice.Get(), "GS", "gs_5_0"));

	HR(m_pEffectHelper->CreateShaderFromFile("Tessellation_PS", L"HLSL\\Tessellation_PS.hlsl",
		m_pd3dDevice.Get(), "PS", "ps_5_0"));

	EffectPassDesc passDesc;
	passDesc.nameVS = "Tessellation_Transform_VS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_NoTess", m_pd3dDevice.Get(), &passDesc));

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameGS = "Tessellation_Point2Square_GS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_Point2Square", m_pd3dDevice.Get(), &passDesc));

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_Isoline_HS";
	passDesc.nameDS = "Tessellation_BezierCurve_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_BezierCurve", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_BezierCurve")->SetRasterizerState(m_pRSWireFrame.Get());

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_Triangle_HS";
	passDesc.nameDS = "Tessellation_Triangle_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_Triangle", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_Triangle")->SetRasterizerState(m_pRSWireFrame.Get());

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_Quad_Integer_HS";
	passDesc.nameDS = "Tessellation_Quad_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_Quad_Integer", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_Quad_Integer")->SetRasterizerState(m_pRSWireFrame.Get());

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_Quad_Odd_HS";
	passDesc.nameDS = "Tessellation_Quad_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_Quad_Odd", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_Quad_Odd")->SetRasterizerState(m_pRSWireFrame.Get());

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_Quad_Even_HS";
	passDesc.nameDS = "Tessellation_Quad_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_Quad_Even", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_Quad_Even")->SetRasterizerState(m_pRSWireFrame.Get());

	passDesc.nameVS = "Tessellation_VS";
	passDesc.nameHS = "Tessellation_BezierSurface_HS";
	passDesc.nameDS = "Tessellation_BezierSurface_DS";
	passDesc.namePS = "Tessellation_PS";
	HR(m_pEffectHelper->AddEffectPass("Tessellation_BezierSurface", m_pd3dDevice.Get(), &passDesc));
	m_pEffectHelper->GetEffectPass("Tessellation_BezierSurface")->SetRasterizerState(m_pRSWireFrame.Get());

	m_pEffectHelper->SetDebugObjectName("Tessellation");

	return true;
}