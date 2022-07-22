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

	m_GpuTimer_Instancing.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
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
	// 获取子类
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

	ImGuiIO& io = ImGui::GetIO();
	// ******************
	// 自由摄像机的操作
	//
	float d1 = 0.0f, d2 = 0.0f;
	if (ImGui::IsKeyDown(ImGuiKey_W))
		d1 += dt;
	if (ImGui::IsKeyDown(ImGuiKey_S))
		d1 -= dt;
	if (ImGui::IsKeyDown(ImGuiKey_A))
		d2 -= dt;
	if (ImGui::IsKeyDown(ImGuiKey_D))
		d2 += dt;

	cam1st->MoveForward(d1 * 6.0f);
	cam1st->Strafe(d2 * 6.0f);

	// 将位置限制在[-119.9f, 119.9f]的区域内
	// 不允许穿地
	XMFLOAT3 adjustedPad;
	XMStoreFloat3(&adjustedPad, XMVectorClamp(cam1st->GetPositionXM(),
		XMVectorSet(-119.9f, 0.0f, -119.9f, 0.0f), XMVectorSet(119.9f, 99.9f, 119.9f, 0.0f)));
	cam1st->SetPosition(adjustedPad);

	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		cam1st->Pitch(io.MouseDelta.y * 0.01f);
		cam1st->RotateY(io.MouseDelta.x * 0.01f);
	}

	if (ImGui::Begin("Instancing and Frustum Culling"))
	{
		static const char* draw_strs[] = { "Trees", "Cubes" };
		if (ImGui::Combo("Scene", &m_SceneMode, draw_strs, ARRAYSIZE(draw_strs)))
		{
			m_GpuTimer_Instancing.Reset(m_pd3dImmediateContext.Get());
		}
		if (ImGui::Checkbox("Enable Instancing", &m_EnableInstancing))
		{
			m_GpuTimer_Instancing.Reset(m_pd3dImmediateContext.Get());
		}
		if (ImGui::Checkbox("Enable Frustum Culling", &m_EnableFrustumCulling))
		{
			m_GpuTimer_Instancing.Reset(m_pd3dImmediateContext.Get());
		}
	}
	ImGui::End();

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

	float gray[4] = { 0.6f,0.6f,0.6f,1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), gray);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
	D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	// 统计实际绘制的物体数目
	// 是否开启视锥体裁剪
	auto& instancedData = (m_SceneMode == 0 ? m_TreeInstancedData : m_CubeInstancedData);
	auto& boundingBox = (m_SceneMode == 0 ? m_Trees.GetModel()->boundingbox : m_Cubes.GetModel()->boundingbox);
	const auto& refTransforms = (m_SceneMode == 0 ? m_TreeTransforms : m_CubeTransforms);
	auto& refObject = (m_SceneMode == 0 ? m_Trees : m_Cubes);

	if (m_EnableFrustumCulling)
	{
		//TODO:
	}

	uint32_t drawCount;
	uint32_t objectCount;


	if (ImGui::Begin("Instancing and Frustum Culling"))
	{
		ImGui::Text("Objects: %u/%u", drawCount, objectCount);

		m_GpuTimer_Instancing.TryGetTime(nullptr);
		double avgTime = m_GpuTimer_Instancing.AverageTime();

		ImGui::Text("Instance Pass: %.3fms", avgTime * 1000.0);
	}
	ImGui::End();
	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}



bool GameApp::InitResource()
{
	// 初始化游戏对象

	m_AcceptedData.reserve(2048);
	m_AcceptedIndicse.reserve(2048);
	m_pInstancedBuffer = std::make_unique<Buffer>(m_pd3dDevice.Get(),
		CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * 2048, D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));

	CreateRandomTrees();
	CreateRandomCubes();

	// 初始化地板
	Model* pModel = m_ModelManager.CreateFromFile(".\\Model\\ground_19.obj");
	m_Ground.SetModel(pModel);
	pModel->SetDebugObjectName("ground_19");

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
	camera->LookTo(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

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

	m_pInstancedBuffer->SetDebugObjectName("InstancedBuffer");

	return true;
}

void GameApp::CreateRandomTrees()
{
	// 初始化树
	Model* pModel = m_ModelManager.CreateFromFile(".\\Model\\tree.obj");
	m_Trees.SetModel(pModel);
	pModel->SetDebugObjectName("Trees");
	XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);

	BoundingBox treeBox = m_Trees.GetModel()->boundingbox;

	// 让树木底部紧贴地面位于 y = -2 的平面
	treeBox.Transform(treeBox, S);
	float Ty = -(treeBox.Center.y - treeBox.Extents.y + 2.0f);
	// 随机生成256颗随机朝向的树
	m_TreeInstancedData.resize(256);
	m_TreeTransforms.resize(256);

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_real<float> radiusNorDist(0.0f, 30.0f);
	std::uniform_real<float> normDist;
	float theta = 0.0f;
	int pos = 0;
	Transform transform;
	transform.SetScale(0.015f, 0.015f, 0.015f);
	for (int i = 0; i < 16; ++i)
	{
		// 取5-125的半径放置随机的树
		for (int j = 0; j < 4; ++j)
		{
			// 距离越远，树越多
			for (int k = 0; k < 2 * j + 1; ++k, ++pos)
			{
				float radius = (float)(radiusNorDist(rng) + 30 * j + 5);
				float randomRad = normDist(rng) * XM_2PI / 16;
				transform.SetRotation(0.0f, normDist(rng) * XM_2PI, 0.0f);
				transform.SetPosition(radius * cosf(theta + randomRad), Ty, radius * sinf(theta + randomRad));

				m_TreeTransforms[pos] = transform;

				XMMATRIX W = transform.GetLocalToWorldMatrixXM();
				XMMATRIX WInvT = XMath::InverseTranspose(W);
				XMStoreFloat4x4(&m_TreeInstancedData[pos].world, XMMatrixTranspose(W));
				XMStoreFloat4x4(&m_TreeInstancedData[pos].worldInvTranspose, XMMatrixTranspose(WInvT));
			}
		}
		theta += XM_2PI / 16;
	}
}

void GameApp::CreateRandomCubes()
{
	// 初始化立方体
	Model* pModel = m_ModelManager.CreateFromGeometry("Cubes", Geometry::CreateBox());
	m_Cubes.SetModel(pModel);
	pModel->SetDebugObjectName("Cubes");

	// 随机生成2048个立方体
	m_CubeInstancedData.resize(2048);
	m_CubeTransforms.resize(2048);

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_real<float> radiusNorDist(0.0f, 40.0f);
	std::uniform_real<float> scaleNorDist(0.25f, 3.0f);
	std::uniform_real<float> heightNorDist(-30.0f, 70.0f);
	std::uniform_real<float> normDist;
	float theta = 0.0f;
	int pos = 0;
	Transform transform;
	transform.SetScale(0.015f, 0.015f, 0.015f);
	for (int i = 0; i < 16; ++i)
	{
		// 取5-165的半径放置随机的立方体
		for (int j = 0; j < 4; ++j)
		{
			// 距离越远，立方体越多
			for (int k = 0; k < 16 * j + 8; ++k, ++pos)
			{
				float radius = (float)(radiusNorDist(rng) + 40 * j + 5);
				float randomRad = normDist(rng) * XM_2PI / 16;
				float scale = scaleNorDist(rng);
				transform.SetScale(scale, scale, scale);
				transform.SetRotation(normDist(rng) * XM_2PI, normDist(rng) * XM_2PI, normDist(rng) * XM_2PI);
				transform.SetPosition(radius * cosf(theta + randomRad), heightNorDist(rng), radius * sinf(theta + randomRad));

				m_CubeTransforms[pos] = transform;

				XMMATRIX W = transform.GetLocalToWorldMatrixXM();
				XMMATRIX WInvT = XMath::InverseTranspose(W);
				XMStoreFloat4x4(&m_CubeInstancedData[pos].world, XMMatrixTranspose(W));
				XMStoreFloat4x4(&m_CubeInstancedData[pos].worldInvTranspose, XMMatrixTranspose(WInvT));
				m_CubeInstancedData[pos].color = XMFLOAT4(normDist(rng), normDist(rng), normDist(rng), normDist(rng));
			}
		}
		theta += XM_2PI / 16;
	}
}
