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

	if (!m_ShadowEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_SSAOEffect.InitAll(m_pd3dDevice.Get()))
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
	m_pDebugAOTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth / 2, m_ClientHeight / 2, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_SSAOManager.OnResize(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);

	m_pDepthTexture->SetDebugObjectName("DepthTexture");
	m_pLitTexture->SetDebugObjectName("LitTexture");
	m_pDebugAOTexture->SetDebugObjectName("DebugAOTexture");

	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_SSAOEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	m_CameraController.Update(dt);

	if (ImGui::Begin("Displacement Mapping"))
	{
		ImGui::Checkbox("Animate Light", &m_UpdateLight);
		static int curr_item = 2;
		static const char* modes[] = {
			"Basic",
			"Normal Map",
			"Displacement Map"
		};

		if (ImGui::Combo("Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			m_RenderMode = static_cast<RenderMode>(curr_item);
		}
		if (m_RenderMode == RenderMode::DisplacementMap)
		{
			ImGui::SliderInt("Height Scale", &m_HeightScale, 0, 15);
		}
		static bool isWireframe = false;
		if (ImGui::Checkbox("Wireframe Mode", &isWireframe))
		{
			m_RasterizerMode = static_cast<RasterizerMode>(isWireframe);
		}
		if (ImGui::Checkbox("Enable SSAO", &m_EnableSSAO))
		{
			m_BasicEffect.SetSSAOEnabled(m_EnableSSAO);
			if (m_EnableDebug && !m_EnableSSAO)
				m_EnableDebug = false;
		}

		if (m_EnableSSAO)
			ImGui::Checkbox("Debug SSAO", &m_EnableDebug);

	}
	ImGui::End();

	m_BasicEffect.SetHeightScale(m_HeightScale / 100.0f);
	m_ShadowEffect.SetHeightScale(m_HeightScale / 100.0f);
	m_SSAOEffect.SetHeightScale(m_HeightScale / 100.0f);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_SSAOEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
	m_ShadowEffect.SetEyePos(m_pCamera->GetPosition());
	m_SSAOEffect.SetEyePos(m_pCamera->GetPosition());

	// 更新光照
	static float theta = 0;
	if (m_UpdateLight)
	{
		theta += dt * XM_2PI / 40.0f;
	}
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR dirVec = XMLoadFloat3(&m_OriginalLightDirs[i]);
		dirVec = XMVector3Transform(dirVec, XMMatrixRotationY(theta));
		XMStoreFloat3(&m_DirLights[i].direction, dirVec);
		m_BasicEffect.SetDirLight(i, m_DirLights[i]);
	}

	// 投影区域为正方体，以原点为中心，以方向光为 +Z 朝向
	XMVECTOR dirVec = XMLoadFloat3(&m_DirLights[0].direction);
	XMMATRIX LightView = XMMatrixLookAtLH(dirVec * 20.0f * (-2.0f), g_XMZero, g_XMIdentityR1);
	m_ShadowEffect.SetViewMatrix(LightView);

	// 将NDC空间 [-1, +1]^2 变换到纹理坐标空间 [0, 1]^2
	static XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// ShadowTransform = V * P * T
	m_BasicEffect.SetShadowTransformMatrix(LightView * XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f) * T);

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

	if (m_EnableSSAO)
	{
		RenderSSAO();
	}
	RenderShadow();
	RenderForward();
	RenderSkybox();

	if (m_EnableDebug)
	{
		if (ImGui::Begin("SSAO Buffer", &m_EnableDebug))
		{
			CD3D11_VIEWPORT vp(0.0f, 0.0f, (float)m_pDebugAOTexture->GetWidth(), (float)m_pDebugAOTexture->GetHeight());
			m_SSAOEffect.RenderAmbientOcclusionToTexture(
				m_pd3dImmediateContext.Get(),
				m_SSAOManager.GetAmbientOcclusionTexture(),
				m_pDebugAOTexture->GetRenderTarget(),
				vp);
			ImVec2 winSize = ImGui::GetWindowSize();
			float smaller = (std::min)((winSize.x - 20) / AspectRatio(), winSize.y - 36);
			ImGui::Image(m_pDebugAOTexture->GetShaderResource(), ImVec2(smaller * AspectRatio(), smaller));
		}
		ImGui::End();
	}
	ImGui::Render();

	ID3D11RenderTargetView* pRTVs[]{ GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

}

void GameApp::RenderSSAO()
{
	// Pass1 绘制场景 生成法线深度图
	m_SSAOManager.Begin(m_pd3dImmediateContext.Get(), m_pDepthTexture->GetDepthStencil(), m_pCamera->GetViewPort());
	{
		if (m_RenderMode == RenderMode::DisplacementMap)
		{
			m_SSAOEffect.SetRenderNormalDepthMapWithDisplacement(m_pd3dImmediateContext.Get());
		}
		else
		{
			m_SSAOEffect.SetRenderNormalDepthMap(m_pd3dImmediateContext.Get());
		}
		m_SSAOEffect.SetRasterizerMode(m_RasterizerMode);
		DrawScene<SSAOEffect>(m_SSAOEffect, [&](SSAOEffect& effect, ID3D11DeviceContext* deviceContext) {
			effect.SetRenderNormalDepthMap(deviceContext);
			effect.SetRasterizerMode(m_RasterizerMode);
		});
	}
	m_SSAOManager.End(m_pd3dImmediateContext.Get());
	// Pass2 生产AO
	m_SSAOManager.RenderToSSAOTexture(m_pd3dImmediateContext.Get(), m_SSAOEffect, *m_pCamera);

	// Pass3 混合
	m_SSAOManager.BlurAmbientMap(m_pd3dImmediateContext.Get(), m_SSAOEffect);
}

void GameApp::RenderShadow()
{
	CD3D11_VIEWPORT shadowViewport(0.0f, 0.0f, (float)m_pShadowMapTexture->GetWidth(), (float)m_pShadowMapTexture->GetHeight());
	m_pd3dImmediateContext->ClearDepthStencilView(m_pShadowMapTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, m_pShadowMapTexture->GetDepthStencil());
	m_pd3dImmediateContext->RSSetViewports(1, &shadowViewport);

	m_ShadowEffect.SetRenderDepthOnly();
	DrawScene(m_ShadowEffect);
}

void GameApp::RenderForward()
{
	float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	ID3D11RenderTargetView* pRTVs[]{ m_pLitTexture->GetRenderTarget() };
	m_pd3dImmediateContext->ClearRenderTargetView(pRTVs[0], black);
	if (!m_EnableSSAO)
	{
		m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	D3D11_VIEWPORT vp = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &vp);

	m_BasicEffect.SetTextureShadowMap(m_pShadowMapTexture->GetShaderResource());
	m_BasicEffect.SetTextureAmbientOcclusion(m_EnableSSAO ? m_SSAOManager.GetAmbientOcclusionTexture() : nullptr);

	switch (m_RenderMode)
	{
	case GameApp::RenderMode::Basic:
		m_BasicEffect.SetRenderDefault();
		break;
	case GameApp::RenderMode::NormalMap:
		m_BasicEffect.SetRenderWithNormalMap();
		break;
	case GameApp::RenderMode::DisplacementMap:
		m_BasicEffect.SetRenderWithDisplacementMap();
		break;
	}

	m_BasicEffect.SetRasterizerMode(m_RasterizerMode);
	DrawScene<BasicEffect>(m_BasicEffect, [&](BasicEffect& effect, ID3D11DeviceContext* deviceContext) {
		effect.SetRenderDefault();
		effect.SetRasterizerMode(m_RasterizerMode);
	});

	m_BasicEffect.SetTextureShadowMap(nullptr);
	m_BasicEffect.SetTextureAmbientOcclusion(nullptr);
	m_BasicEffect.Apply(m_pd3dImmediateContext.Get());
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
	m_CameraController.InitCamera(m_pCamera.get());

	// 初始化阴影贴图和特效
	m_pShadowMapTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), 2048, 2048);

	m_pShadowMapTexture->SetDebugObjectName("ShadowMapTexture");

	m_BasicEffect.SetDepthBias(0.005f);
	m_BasicEffect.SetSSAOEnabled(m_EnableSSAO);
	m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());		//忘记设置投影矩阵了
	m_BasicEffect.SetHeightScale(0.07f);
	m_BasicEffect.SetTessInfo(1.0f, 25.0f, 1.0f, 5.0f);

	m_SSAOEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_SSAOEffect.SetProjMatrix(camera->GetProjMatrixXM());
	m_SSAOEffect.SetHeightScale(0.07f);
	m_SSAOEffect.SetTessInfo(1.0f, 25.0f, 1.0f, 5.0f);

	m_ShadowEffect.SetProjMatrix(XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f));
	m_ShadowEffect.SetHeightScale(0.007f);
	m_ShadowEffect.SetTessInfo(1.0f, 25.0f, 1.0f, 5.0f);

	m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());

	m_SSAOManager.InitResource(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);

	// 初始化对象
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Ground",
			Geometry::CreateGrid(XMFLOAT2(20.0f, 30.0f), XMUINT2(40, 50), XMFLOAT2(6.0f, 9.0f)));
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

	return true;
}