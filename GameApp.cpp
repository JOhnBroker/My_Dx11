#include "GameApp.h"
#include "XUtil.h"
#include "DXTrace.h"
#include <DirectXColors.h>
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	m_TextureManager.Init(m_pd3dDevice.Get());
	m_ModelManager.Init(m_pd3dDevice.Get());

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	//if (!m_ShadowEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;
	//
	//if (!m_SSAOEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;

	if (!m_FireEffect.InitAll(m_pd3dDevice.Get(), L"HLSL\\Fire.hlsl"))
		return false;
	if (!m_RainEffect.InitAll(m_pd3dDevice.Get(), L"HLSL\\Rain.hlsl"))
		return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();

	m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
	m_pLitTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	
	m_pDepthTexture->SetDebugObjectName("DepthTexture");
	m_pLitTexture->SetDebugObjectName("LitTexture");

	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_FireEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_RainEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	//m_CameraController.Update(dt);
	//
	//if (ImGui::Begin("Displacement Mapping"))
	//{
	//	ImGui::Checkbox("Animate Light", &m_UpdateLight);
	//	static int curr_item = 2;
	//	static const char* modes[] = {
	//		"Basic",
	//		"Normal Map",
	//		"Displacement Map"
	//	};
	//
	//	if (ImGui::Combo("Mode", &curr_item, modes, ARRAYSIZE(modes)))
	//	{
	//		m_RenderMode = static_cast<RenderMode>(curr_item);
	//	}
	//	if (m_RenderMode == RenderMode::DisplacementMap)
	//	{
	//		ImGui::SliderInt("Height Scale", &m_HeightScale, 0, 15);
	//	}
	//	static bool isWireframe = false;
	//	if (ImGui::Checkbox("Wireframe Mode", &isWireframe))
	//	{
	//		m_RasterizerMode = static_cast<RasterizerMode>(isWireframe);
	//	}
	//	if (ImGui::Checkbox("Enable SSAO", &m_EnableSSAO))
	//	{
	//		m_BasicEffect.SetSSAOEnabled(m_EnableSSAO);
	//		if (m_EnableDebug && !m_EnableSSAO)
	//			m_EnableDebug = false;
	//	}
	//
	//	if (m_EnableSSAO)
	//		ImGui::Checkbox("Debug SSAO", &m_EnableDebug);
	//
	//}
	//ImGui::End();
	//
	//m_BasicEffect.SetHeightScale(m_HeightScale / 100.0f);
	//m_ShadowEffect.SetHeightScale(m_HeightScale / 100.0f);
	//m_SSAOEffect.SetHeightScale(m_HeightScale / 100.0f);
	//
	//m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//m_SSAOEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//
	//m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
	//m_ShadowEffect.SetEyePos(m_pCamera->GetPosition());
	//m_SSAOEffect.SetEyePos(m_pCamera->GetPosition());
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

	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

	ImGuiIO& io = ImGui::GetIO();
	// ******************
	// 第一人称摄像机的操作
	//
	float d1 = 0.0f, d2 = 0.0f;
	if (ImGui::IsKeyDown('W'))
		d1 += dt;
	if (ImGui::IsKeyDown('S'))
		d1 -= dt;
	if (ImGui::IsKeyDown('A'))
		d2 -= dt;
	if (ImGui::IsKeyDown('D'))
		d2 += dt;

	cam1st->Walk(d1 * 6.0f);
	cam1st->Strafe(d2 * 6.0f);

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		cam1st->Pitch(io.MouseDelta.y * 0.01f);
		cam1st->RotateY(io.MouseDelta.x * 0.01f);
	}

	if (ImGui::Begin("Particle System"))
	{
		if (ImGui::Button("Reset Particle"))
		{
			m_Fire.Reset();
			m_Rain.Reset();
		}
	}
	ImGui::End();
	ImGui::Render();

	// 将位置限制在[-80.0f, 80.0f]的区域内
	// 不允许穿地
	XMFLOAT3 adjustedPos;
	XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorReplicate(-80.0f), XMVectorReplicate(80.0f)));
	cam1st->SetPosition(adjustedPos);

	m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

	m_Fire.Update(dt, m_Timer.TotalTime());
	m_Rain.Update(dt, m_Timer.TotalTime());

	m_FireEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_FireEffect.SetEyePos(m_pCamera->GetPosition());
	//m_RainEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	//m_RainEffect.SetEyePos(m_pCamera->GetPosition());

	static XMFLOAT3 lastCameraPos = m_pCamera->GetPosition();
	XMFLOAT3 cameraPos = m_pCamera->GetPosition();

	XMVECTOR cameraPosVec = XMLoadFloat3(&cameraPos);
	XMVECTOR lastCameraPosVec = XMLoadFloat3(&lastCameraPos);
	XMFLOAT3 emitPos;
	XMStoreFloat3(&emitPos, cameraPosVec + 3.0f * (cameraPosVec - lastCameraPosVec));
	m_RainEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_RainEffect.SetEyePos(m_pCamera->GetPosition());
	m_Rain.SetEmitPos(emitPos);
	lastCameraPos = m_pCamera->GetPosition();
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

	float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(m_pLitTexture->GetRenderTarget(), black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* pRTVs[]{ m_pLitTexture->GetRenderTarget() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	D3D11_VIEWPORT vp = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &vp);

	m_BasicEffect.DrawInstanced(m_pd3dImmediateContext.Get(), *m_pInstancedBuffer, m_Trees, 144);
	m_BasicEffect.SetRenderDefault();
	m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	pRTVs[0] = GetBackBufferRTV();
	m_pd3dImmediateContext->RSSetViewports(1, &vp);
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	m_SkyboxEffect.SetRenderDefault();
	m_SkyboxEffect.SetDepthTexture(m_pDepthTexture->GetShaderResource());
	m_SkyboxEffect.SetLitTexture(m_pLitTexture->GetShaderResource());
	m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
	m_SkyboxEffect.SetDepthTexture(nullptr);
	m_SkyboxEffect.SetLitTexture(nullptr);
	m_SkyboxEffect.Apply(m_pd3dImmediateContext.Get());

	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	m_Fire.Draw(m_pd3dImmediateContext.Get(), m_FireEffect);
	m_Rain.Draw(m_pd3dImmediateContext.Get(), m_RainEffect);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

}

void GameApp::RenderSSAO()
{
	//// Pass1 绘制场景 生成法线深度图
	//m_SSAOManager.Begin(m_pd3dImmediateContext.Get(), m_pDepthTexture->GetDepthStencil(), m_pCamera->GetViewPort());
	//{
	//	if (m_RenderMode == RenderMode::DisplacementMap)
	//	{
	//		m_SSAOEffect.SetRenderNormalDepthMapWithDisplacement(m_pd3dImmediateContext.Get());
	//	}
	//	else
	//	{
	//		m_SSAOEffect.SetRenderNormalDepthMap(m_pd3dImmediateContext.Get());
	//	}
	//	m_SSAOEffect.SetRasterizerMode(m_RasterizerMode);
	//	DrawScene<SSAOEffect>(m_SSAOEffect, [&](SSAOEffect& effect, ID3D11DeviceContext* deviceContext) {
	//		effect.SetRenderNormalDepthMap(deviceContext);
	//		effect.SetRasterizerMode(m_RasterizerMode);
	//	});
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
	//DrawScene(m_ShadowEffect);
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
	//switch (m_RenderMode)
	//{
	//case GameApp::RenderMode::Basic:
	//	m_BasicEffect.SetRenderDefault();
	//	break;
	//case GameApp::RenderMode::NormalMap:
	//	m_BasicEffect.SetRenderWithNormalMap();
	//	break;
	//case GameApp::RenderMode::DisplacementMap:
	//	m_BasicEffect.SetRenderWithDisplacementMap();
	//	break;
	//}
	//
	//m_BasicEffect.SetRasterizerMode(m_RasterizerMode);
	//DrawScene<BasicEffect>(m_BasicEffect, [&](BasicEffect& effect, ID3D11DeviceContext* deviceContext) {
	//	effect.SetRenderDefault();
	//	effect.SetRasterizerMode(m_RasterizerMode);
	//});
	//
	//m_BasicEffect.SetTextureShadowMap(nullptr);
	//m_BasicEffect.SetTextureAmbientOcclusion(nullptr);
	//m_BasicEffect.Apply(m_pd3dImmediateContext.Get());
}
void GameApp::RenderSkybox()
{
	D3D11_VIEWPORT skyboxViewport = m_pCamera->GetViewPort();
	skyboxViewport.MinDepth = 1.0f;
	skyboxViewport.MaxDepth = 1.0f;
	m_pd3dImmediateContext->RSSetViewports(1, &skyboxViewport);

	m_SkyboxEffect.SetRenderDefault();
	m_SkyboxEffect.SetDepthTexture(m_pDepthTexture->GetShaderResource());
	m_SkyboxEffect.SetLitTexture(m_pLitTexture->GetShaderResource());

	// 由于全屏绘制，不需要用到深度缓冲区，也就不需要清空后备缓冲区了
	ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);

	m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_SkyboxEffect.SetDepthTexture(nullptr);
	m_SkyboxEffect.SetLitTexture(nullptr);
	m_SkyboxEffect.Apply(m_pd3dImmediateContext.Get());
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

	// 初始化

	m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());		//忘记设置投影矩阵了
	
	m_FireEffect.SetBlendState(RenderStates::BSAlphaWeightedAdditive.Get(), nullptr, 0xFFFFFFFF);
	m_FireEffect.SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
	m_FireEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_FireEffect.SetProjMatrix(camera->GetProjMatrixXM());

	m_RainEffect.SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
	m_RainEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_RainEffect.SetProjMatrix(camera->GetProjMatrixXM());

	m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());

	// 初始化对象

	CreateRandomTrees();
	
	Model* pModel = m_ModelManager.CreateFromFile("Model\\ground_35.obj");
	pModel->SetDebugObjectName("Ground");
	m_Ground.SetModel(pModel);
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
		pModel->SetDebugObjectName("Skybox");
		m_Skybox.SetModel(pModel);
		m_TextureManager.CreateFromFile("Texture\\grasscube1024.dds", false, true);
		pModel->materials[0].Set<std::string>("$Skybox", "Texture\\grasscube1024.dds");
	}

	m_TextureManager.CreateFromFile("Texture\\flare0.dds", false, true);
	m_TextureManager.CreateFromFile("Texture\\raindrop.dds", false, true);

	std::mt19937 randEngine;
	randEngine.seed(std::random_device()());
	std::uniform_real_distribution<float> randF(-1.0f, 1.0f);
	std::vector<float> randomValues(4096);

	CD3D11_TEXTURE1D_DESC texDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, 1024, 1, 1);
	D3D11_SUBRESOURCE_DATA initData{ randomValues.data(),1024 * GetFormatSize(DXGI_FORMAT_R32G32B32A32_FLOAT) };
	ComPtr<ID3D11Texture1D> pRandomTex;
	ComPtr<ID3D11ShaderResourceView> pRandomTexSRV;

	std::generate(randomValues.begin(), randomValues.end(), [&]() {return randF(randEngine); });
	HR(m_pd3dDevice->CreateTexture1D(&texDesc, &initData, pRandomTex.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateShaderResourceView(pRandomTex.Get(), nullptr, pRandomTexSRV.ReleaseAndGetAddressOf()));
	m_TextureManager.AddTexture("FireRandomTex", pRandomTexSRV.Get());

	m_Fire.InitResource(m_pd3dDevice.Get(), 500);
	m_Fire.SetTextureInput(m_TextureManager.GetTexture("Texture\\flare0.dds"));
	m_Fire.SetTextureRandom(m_TextureManager.GetTexture("FireRandomTex"));
	m_Fire.SetEmitPos(XMFLOAT3(0.0f, -1.0f, 0.0f));
	m_Fire.SetEmitDir(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_Fire.SetAcceleration(XMFLOAT3(0.0f, 7.8f, 0.0f));
	m_Fire.SetEmitInterval(0.005f);
	m_Fire.SetAliveTime(1.0f);
	m_Fire.SetDebugObjectName("Fire");

	std::generate(randomValues.begin(), randomValues.end(), [&]() {return randF(randEngine); });
	HR(m_pd3dDevice->CreateTexture1D(&texDesc, &initData, pRandomTex.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateShaderResourceView(pRandomTex.Get(), nullptr, pRandomTexSRV.ReleaseAndGetAddressOf()));
	m_TextureManager.AddTexture("RainRandomTex", pRandomTexSRV.Get());

	m_Rain.InitResource(m_pd3dDevice.Get(), 10000);
	m_Rain.SetTextureInput(m_TextureManager.GetTexture("Texture\\raindrop.dds"));
	m_Rain.SetTextureRandom(m_TextureManager.GetTexture("RainRandomTex"));
	m_Rain.SetEmitDir(XMFLOAT3(0.0f, -1.0f, 0.0f));
	m_Rain.SetAcceleration(XMFLOAT3(-1.0f, -9.8f, 0.0f));
	m_Rain.SetEmitInterval(0.0015f);
	m_Rain.SetAliveTime(3.0f);
	m_Rain.SetDebugObjectName("Rain");

	// ******************
	// 初始化不会变化的值
	//

	// 环境光
	DirectionalLight dirLight[4];
	dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	dirLight[0].diffuse = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	dirLight[1] = dirLight[0];
	dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
	dirLight[2] = dirLight[0];
	dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
	dirLight[3] = dirLight[0];
	dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
	for (int i = 0; i < 4; ++i)
	{
		m_BasicEffect.SetDirLight(i, dirLight[i]);
	}

	return true;
}

void GameApp::CreateRandomTrees()
{
	// 初始化树
	Model* pModel = m_ModelManager.CreateFromFile("Model\\tree.obj");
	pModel->SetDebugObjectName("Trees");
	m_Trees.SetModel(pModel);
	XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);

	BoundingBox treeBox = m_Trees.GetModel()->boundingbox;

	// 让树木底部紧贴地面位于y = -2的平面
	treeBox.Transform(treeBox, S);
	float Ty = -(treeBox.Center.y - treeBox.Extents.y + 2.0f);
	// 随机生成144颗随机朝向的树
	std::vector<BasicEffect::InstancedData> treeData(144);
	m_pInstancedBuffer = std::make_unique<Buffer>(m_pd3dDevice.Get(),
		CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * 144, D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
	m_pInstancedBuffer->SetDebugObjectName("InstancedBuffer");

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_real<float> radiusNormDist(0.0f, 30.0f);
	std::uniform_real<float> normDist;
	float theta = 0.0f;
	int pos = 0;
	Transform transform;
	transform.SetScale(0.015f, 0.015f, 0.015f);
	for (int i = 0; i < 16; ++i)
	{
		// 取5-95的半径放置随机的树
		for (int j = 0; j < 3; ++j)
		{
			// 距离越远，树木越多
			for (int k = 0; k < 2 * j + 1; ++k, ++pos)
			{
				float radius = (float)(radiusNormDist(rng) + 30 * j + 5);
				float randomRad = normDist(rng) * XM_2PI / 16;
				transform.SetRotation(0.0f, normDist(rng) * XM_2PI, 0.0f);
				transform.SetPosition(radius * cosf(theta + randomRad), Ty, radius * sinf(theta + randomRad));

				XMStoreFloat4x4(&treeData[pos].world,
					XMMatrixTranspose(transform.GetLocalToWorldMatrixXM()));
				XMStoreFloat4x4(&treeData[pos].worldInvTranspose,
					XMMatrixTranspose(XMath::InverseTranspose(transform.GetLocalToWorldMatrixXM())));
			}
		}
		theta += XM_2PI / 16;
	}

	memcpy_s(m_pInstancedBuffer->MapDiscard(m_pd3dImmediateContext.Get()), m_pInstancedBuffer->GetByteWidth(),
		treeData.data(), treeData.size() * sizeof(BasicEffect::InstancedData));
	m_pInstancedBuffer->Unmap(m_pd3dImmediateContext.Get());
}
