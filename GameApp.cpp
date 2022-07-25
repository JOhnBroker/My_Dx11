#include "GameApp.h"
#include "XUtil.h"
#include "DXTrace.h"
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

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();

	m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
	m_pDepthTexture->SetDebugObjectName("DepthTexture");

	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	m_CameraController.Update(dt);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

	m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

	if (ImGui::Begin("Static Cube Mapping"))
	{
		static int skybox_item = 0;
		static const char* skybox_strs[] = {
			"Daylight",
			"Sunset",
			"Desert"
		};
		if (ImGui::Combo("Skybox", &skybox_item, skybox_strs, ARRAYSIZE(skybox_strs)))
		{
			Model* pModel = m_ModelManager.GetModel("Skybox");
			switch (skybox_item)
			{
			case 0:
				m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("DayLight"));
				pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
				break;
			case 1:
				m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Sunset"));
				pModel->materials[0].Set<std::string>("$Skybox", "Sunset");
				break;
			case 2:
				m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Desert"));
				pModel->materials[0].Set<std::string>("$Skybox", "Desert");
				break;
			}
		}
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

	float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	m_BasicEffect.SetRenderDefault();
	m_BasicEffect.SetReflectionEnable(true);
	m_Sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetReflectionEnable(false);
	m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Cylinder.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 绘制天空盒
	m_SkyboxEffect.SetRenderDefault();
	m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
	// 初始化游戏对象

	// 初始化


	// ******************
	// 初始化摄像机
	//

	/*auto camera = std::make_shared<ThirdPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	camera->SetDistance(15.0f);
	camera->SetDistanceMinMax(6.0f, 100.0f);
	camera->SetRotationX(XM_PIDIV4);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);*/

	auto camera = std::make_shared<FirstPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());

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
		m_BasicEffect.SetDirLight(i, dirLight[i]);

	m_BasicEffect.SetRenderDefault();

	return true;
}
