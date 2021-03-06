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
		m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImGuiIO& io = ImGui::GetIO();
	static int mouseStatus = 0;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		if (mousePos.x >= m_DebugTextureXY.x && mousePos.x < m_DebugTextureXY.x + m_DebugTextureWH.x &&
			mousePos.y >= m_DebugTextureXY.y && mousePos.y < m_DebugTextureXY.y + m_DebugTextureWH.y)
			mouseStatus = 1;
		else
			mouseStatus = 0;
	}

	if (mouseStatus == 1)
	{
		float yaw = 0.0f, pitch = 0.0f;
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{

			yaw += io.MouseDelta.x * 0.015f;
			pitch += io.MouseDelta.y * 0.015f;
		}
		m_pDebugCamera->RotateY(yaw);
		m_pDebugCamera->Pitch(pitch);
	}
	else
	{
		m_CameraController.Update(dt);
	}

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

	m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

	if (ImGui::Begin("Dynamic Cube Mapping"))
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
				m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
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

		static int sphere_item = static_cast<int>(m_SphereMode);
		static const char* sphere_modes[] =
		{
			"None",
			"Reflection",
			"Refraction"
		};
		if (ImGui::Combo("Sphere Mode", &sphere_item, sphere_modes, ARRAYSIZE(sphere_modes)))
		{
			m_SphereMode = static_cast<SphereMode>(sphere_item);
		}
		if (sphere_item == 2)
		{
			if (ImGui::SliderFloat("Eta", &m_Eta, 0.2f, 1.0f))
			{
				m_BasicEffect.SetRefractionEta(m_Eta);
			}
		}
		ImGui::Checkbox("Use Geometry Shader", &m_UseGS);
	}
	ImGui::End();
	// 球体动画
	m_SphereRad += dt;
	for (int i = 0; i < 4; ++i)
	{
		auto& transform = m_Sphere[i].GetTransform();
		auto pos = transform.GetPosition();
		pos.y = 0.5f * std::sin(m_SphereRad);
		transform.SetPosition(pos);
	}
	m_Sphere[4].GetTransform().RotateAround(XMFLOAT3(), XMFLOAT3(0.0f, 1.0f, 0.0f), 2.0f * dt);
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
	// 生成动态天空盒
	static XMFLOAT3 ups[6] = {
		{ 0.0f, 1.0f, 0.0f },	// +X
		{ 0.0f, 1.0f, 0.0f },   // -X
		{ 0.0f, 0.0f, -1.0f },  // +Y
		{ 0.0f, 0.0f, 1.0f },   // -Y
		{ 0.0f, 1.0f, 0.0f },   // +Z
		{ 0.0f, 1.0f, 0.0f }    // -Z
	};

	static XMFLOAT3 looks[6] = {
		{ 1.0f, 0.0f, 0.0f },	// +X
		{ -1.0f, 0.0f, 0.0f },  // -X
		{ 0.0f, 1.0f, 0.0f },	// +Y
		{ 0.0f, -1.0f, 0.0f },  // -Y
		{ 0.0f, 0.0f, 1.0f },	// +Z
		{ 0.0f, 0.0f, -1.0f },  // -Z
	};

	if (m_UseGS == false)
	{
		BoundingFrustum frustum;
		BoundingFrustum::CreateFromMatrix(frustum, m_pCamera->GetProjMatrixXM());
		frustum.Transform(frustum, m_pCamera->GetLocalToWorldMatrixXM());

		// 中心球绘制量较大，能不画就不画
		m_CenterSphere.FrustumCulling(frustum);
		m_BasicEffect.SetEyePos(m_CenterSphere.GetTransform().GetPosition());
		if (m_CenterSphere.InFrustum())
		{
			for (int i = 0; i < 6; ++i)
			{
				m_pCubeCamera->LookAt(m_CenterSphere.GetTransform().GetPosition(), looks[i], ups[i]);
				DrawScene(false, false, *m_pCubeCamera, m_pDynamicTextureCube->GetRenderTarget(i),
					m_pDynamicCubeDepthTexture->GetDepthStencil(), m_pDynamicTextureCube->GetShaderResource());
			}
		}
		m_pSRV = m_pDynamicTextureCube->GetShaderResource();
	}
	else
	{
		for (int i = 0; i < 6; ++i)
		{
			m_pCubeCamera->LookAt(m_CenterSphere.GetTransform().GetPosition(), looks[i], ups[i]);
			DirectX::XMMATRIX V = m_pCubeCamera->GetViewMatrixXM();
			DirectX::XMMATRIX P = m_pCubeCamera->GetProjMatrixXM();
			V.r[3] = g_XMIdentityR3;
			V = V * P;
			V = XMMatrixTranspose(V);
			m_SkyboxEffect.SetViewProjMatrixs(V, i);
			m_SkyboxEffect.SetWorldMatrix(m_pCubeCamera->GetLocalToWorldMatrixXM());
			m_BasicEffect.SetViewProjMatrixs(V, i);
		}
		DrawScene(false, m_UseGS, *m_pCubeCamera, m_pDynamicSkyboxGS->GetRenderTarget(),
			m_pDepthArray->GetDepthStencil(), m_pDynamicSkyboxGS->GetShaderResource());
		m_pSRV = m_pDynamicSkyboxGS->GetShaderResource();
	}

	// 绘制场景
	DrawScene(true, m_UseGS, *m_pCamera, GetBackBufferRTV(), m_pDepthTexture->GetDepthStencil(), m_pSRV.Get());

	// 绘制天空盒
	static bool debugCube = false;
	if (ImGui::Begin("Dymamic Cube Mapping"))
	{
		ImGui::Checkbox("Debug Cube", &debugCube);
	}
	ImGui::End();

	m_DebugTextureXY = {};
	m_DebugTextureWH = {};

	if (debugCube)
	{
		if (ImGui::Begin("Debug"))
		{
			D3D11_VIEWPORT viewport = m_pDebugCamera->GetViewPort();
			ID3D11RenderTargetView* pRTVs[]{ m_pDebugDynamicCubeTexture->GetRenderTarget() };
			m_pd3dImmediateContext->RSSetViewports(1, &viewport);
			m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
			m_SkyboxEffect.SetRenderDefault();
			m_SkyboxEffect.SetViewMatrix(m_pDebugCamera->GetViewMatrixXM());
			m_SkyboxEffect.SetProjMatrix(m_pDebugCamera->GetProjMatrixXM());
			m_DebugSkybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
			// 画完后清空
			ID3D11ShaderResourceView* nullSRVs[3]{};
			m_pd3dImmediateContext->PSSetShaderResources(0, 3, nullSRVs);
			// 复原
			viewport = m_pCamera->GetViewPort();
			pRTVs[0] = GetBackBufferRTV();
			m_pd3dImmediateContext->RSSetViewports(1, &viewport);
			m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);

			ImVec2 winSize = ImGui::GetWindowSize();
			float smaller = (std::min)(winSize.x - 20, winSize.y - 36);
			ImGui::Image(m_pDebugDynamicCubeTexture->GetShaderResource(), ImVec2(smaller, smaller));
			m_DebugTextureXY = ImGui::GetItemRectMin();
			m_DebugTextureWH = { smaller, smaller };
		}
		ImGui::End();
	}

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
	// 初始化天空盒
	ComPtr<ID3D11Texture2D> pTex;
	D3D11_TEXTURE2D_DESC texDesc;
	std::string filenameStr;
	std::vector<ID3D11ShaderResourceView*> pCubeTextures;
	std::unique_ptr<TextureCube> pTexCube;
	// Daylight
	{
		filenameStr = "Texture\\daylight0.png";
		for (size_t i = 0; i < 6; ++i)
		{
			filenameStr[16] = '0' + (char)i;
			pCubeTextures.push_back(m_TextureManager.CreateTexture(filenameStr));
		}
		pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
		pTex->GetDesc(&texDesc);
		pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		pTexCube->SetDebugObjectName("Daylight");

		for (uint32_t i = 0; i < 6; ++i)
		{
			pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
			m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(),
				D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
		}
		m_TextureManager.AddTexture("Daylight", pTexCube->GetShaderResource());
	}
	// Sunset
	{
		filenameStr = "Texture\\sunset0.bmp";
		pCubeTextures.clear();
		for (size_t i = 0; i < 6; ++i)
		{
			filenameStr[14] = '0' + (char)i;
			pCubeTextures.push_back(m_TextureManager.CreateTexture(filenameStr));
		}
		pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
		pTex->GetDesc(&texDesc);
		pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		pTexCube->SetDebugObjectName("Sunset");

		for (uint32_t i = 0; i < 6; ++i)
		{
			pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
			m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(),
				D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
		}
		m_TextureManager.AddTexture("Sunset", pTexCube->GetShaderResource());
	}
	// Desert
	m_TextureManager.AddTexture("Desert", m_TextureManager.CreateTexture("Texture\\desertcube1024.dds", false, true));

	// 动态天空盒
	m_pDynamicTextureCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDynamicCubeDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), 256, 256);
	m_pDebugDynamicCubeTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDynamicSkyboxGS = std::make_unique<TextureCube>(m_pd3dDevice.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDepthArray = std::make_unique<Depth2DArray>(m_pd3dDevice.Get(), 256, 256, 6);
	m_TextureManager.AddTexture("DynamicCube", m_pDynamicTextureCube->GetShaderResource());
	m_TextureManager.AddTexture("DynamicCubeGS", m_pDynamicSkyboxGS->GetShaderResource());

	m_pDynamicTextureCube->SetDebugObjectName("DynamicTextureCube");
	m_pDynamicCubeDepthTexture->SetDebugObjectName("DynamicCubeDepthTexture");
	m_pDynamicSkyboxGS->SetDebugObjectName("DynamicTextureCubeGS");
	m_pDepthArray->SetDebugObjectName("DynamicCubeDepthArray");
	m_pDebugDynamicCubeTexture->SetDebugObjectName("DebugDynamicCube");

	// 初始化游戏对象
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Sphere", Geometry::CreateSphere());
		pModel->SetDebugObjectName("Sphere");
		m_TextureManager.CreateTexture("Texture\\stone.dds");
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\stone.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		Transform sphereTransform[] =
		{
			Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, 4.5f)),
			Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, 4.5f)),
			Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, -4.5f)),
			Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, -4.5f)),
			Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(2.5f, 0.0f, 0.0f))
		};
		for (size_t i = 0; i < 5; ++i)
		{
			m_Sphere[i].GetTransform() = sphereTransform[i];
			m_Sphere[i].SetModel(pModel);
		}
		m_CenterSphere.SetModel(pModel);
	}
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreatePlane(XMFLOAT2(10.0f, 10.0f), XMFLOAT2(5.0f, 5.0f)));
		pModel->SetDebugObjectName("Ground");
		m_TextureManager.CreateTexture("Texture\\floor.dds");
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\floor.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
		m_Ground.SetModel(pModel);
		m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);
	}
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Cylinder", Geometry::CreateCylinder(0.5f, 2.0f));
		pModel->SetDebugObjectName("Cylinder");
		m_TextureManager.CreateTexture("Texture\\bricks.dds");
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\bricks.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
		// 需要固定位置
		Transform cylinderTransforms[] = {
			Transform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(0.0f, -1.99f, 0.0f)),
			Transform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(4.5f, -1.99f, 4.5f)),
			Transform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-4.5f, -1.99f, 4.5f)),
			Transform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-4.5f, -1.99f, -4.5f)),
			Transform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(4.5f, -1.99f, -4.5f)),
		};

		for (size_t i = 0; i < 5; ++i)
		{
			m_Cylinder[i].SetModel(pModel);
			m_Cylinder[i].GetTransform() = cylinderTransforms[i];
		}
	}
	// 天空盒立方体
	Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
	pModel->SetDebugObjectName("Skybox");
	pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
	m_Skybox.SetModel(pModel);

	// 调试立方体现在放的是使用GS生成的动态天空盒
	// 调试立方体
	pModel = m_ModelManager.CreateFromGeometry("DebugSkybox", Geometry::CreateBox());
	pModel->SetDebugObjectName("DebugSkybox");
	pModel->materials[0].Set<std::string>("$Skybox", "DynamicCubeGS");
	m_DebugSkybox.SetModel(pModel);

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

	m_pCamera = std::make_shared<FirstPersonCamera>();
	m_CameraController.InitCamera(m_pCamera.get());
	m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	m_pCamera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pCubeCamera = std::make_shared<FirstPersonCamera>();
	m_pCubeCamera->SetFrustum(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);
	m_pCubeCamera->SetViewPort(0.0f, 0.0f, 256.0f, 256.0f);

	m_pDebugCamera = std::make_shared<FirstPersonCamera>();
	m_pDebugCamera->SetFrustum(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);
	m_pDebugCamera->SetViewPort(0.0f, 0.0f, 256.0f, 256.0f);

	// ******************
	// 初始化不会变化的值
	//

	// 环境光
	DirectionalLight dirLight[4]{};
	dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
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

	return true;
}

void GameApp::DrawScene(bool drawCenterSphere, bool enableGS, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, ID3D11ShaderResourceView* pSRV)

{
	float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(pRTV, black);
	m_pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);

	BoundingFrustum frustum;
	D3D11_VIEWPORT viewport = camera.GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	//绘制模型
	m_BasicEffect.SetViewMatrix(camera.GetViewMatrixXM());
	m_BasicEffect.SetProjMatrix(camera.GetProjMatrixXM());
	m_BasicEffect.SetEyePos(camera.GetPosition());
	if (enableGS && !drawCenterSphere)
	{
		m_BasicEffect.SetRenderGS();
	}
	else
	{
		BoundingFrustum::CreateFromMatrix(frustum, camera.GetProjMatrixXM());
		frustum.Transform(frustum, camera.GetLocalToWorldMatrixXM());
		m_BasicEffect.SetRenderDefault();
	}

	// 只有球体才有反射或折射效果
	if (drawCenterSphere)
	{
		switch (m_SphereMode)
		{
		case SphereMode::None:
			m_BasicEffect.SetReflectionEnabled(false);
			m_BasicEffect.SetRefractionEnabled(false);
			break;
		case SphereMode::Reflection:
			m_BasicEffect.SetReflectionEnabled(true);
			m_BasicEffect.SetRefractionEnabled(false);
			break;
		case SphereMode::Refraction:
			m_BasicEffect.SetReflectionEnabled(false);
			m_BasicEffect.SetRefractionEnabled(true);
			break;
		}
		m_BasicEffect.SetTextureCube(pSRV);
		m_CenterSphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		m_BasicEffect.SetTextureCube(nullptr);
	}

	m_BasicEffect.SetReflectionEnabled(false);
	m_BasicEffect.SetRefractionEnabled(false);

	m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	for (auto& cylinder : m_Cylinder)
	{
		if (!enableGS)
			cylinder.FrustumCulling(frustum);
		cylinder.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	for (auto& sphere : m_Sphere)
	{
		if (!enableGS)
			sphere.FrustumCulling(frustum);
		sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}

	// 绘制天空盒
	m_SkyboxEffect.SetViewMatrix(camera.GetViewMatrixXM());
	m_SkyboxEffect.SetProjMatrix(camera.GetProjMatrixXM());
	if (enableGS && !drawCenterSphere)
	{
		m_SkyboxEffect.SetRenderGS();
	}
	else
	{
		m_SkyboxEffect.SetRenderDefault();
	}
	m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
}
