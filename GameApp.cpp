#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: D3DApp(hInstance, windowName, initWidth, initHeight),
	m_ShowMode(Mode::SplitedTriangle),
	m_VertexCount()
	//m_CameraMode(CameraMode::ThirdPerson),
	//m_ShadowMat(),
	//m_WoodCrateMat()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

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

	m_BasicEffect.SetProjMatrix(XMMatrixPerspectiveFovLH(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f));

	//// 摄像机变更显示
	//if (m_pCamera != nullptr)
	//{
	//	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	//	m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	//	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	//}
}

void GameApp::UpdateScene(float dt)
{
	/*
	// 获取子类
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

	ImGuiIO& io = ImGui::GetIO();
	if (m_CameraMode == CameraMode::Free)
	{
		// 第一人称/自由摄像机的操作
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

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			cam1st->Pitch(io.MouseDelta.y * 0.01f);
			cam1st->RotateY(io.MouseDelta.x * 0.01f);
		}
	}
	else if (m_CameraMode == CameraMode::ThirdPerson)
	{
		// 第三人称摄像机的操作
		XMFLOAT3 target = m_BoltAnim.GetTransform().GetPosition();
		cam3rd->SetTarget(target);

		// 绕物体旋转
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			cam3rd->RotateX(io.MouseDelta.y * 0.01f);
			cam3rd->RotateY(io.MouseDelta.x * 0.01f);
		}
		cam3rd->Approach(-io.MouseWheel * 1.0f);
	}

	if (ImGui::Begin("Depth Test"))
	{
		ImGui::Text("W/S/A/D in FPS/Free camera");
		ImGui::Text("Hold the right mouse button and drag the view");

		static int curr_item = 0;
		static const char* modes[] = {
			"Third Person",
			"Free Camera"
		};
		if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			if (curr_item == 0 && m_CameraMode != CameraMode::ThirdPerson)
			{
				if (!cam3rd)
				{
					cam3rd = std::make_shared<ThirdPersonCamera>();
					cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
					m_pCamera = cam3rd;
				}
				XMFLOAT3 target = m_BoltAnim.GetTransform().GetPosition();
				cam3rd->SetTarget(target);
				cam3rd->SetDistance(5.0f);
				cam3rd->SetDistanceMinMax(2.0f, 14.0f);
				cam3rd->SetRotationX(XM_PIDIV4);

				m_CameraMode = CameraMode::ThirdPerson;
			}
			else if (curr_item == 1 && m_CameraMode != CameraMode::Free)
			{
				if (!cam1st)
				{
					cam1st = std::make_shared<FirstPersonCamera>();
					cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
					m_pCamera = cam1st;
				}
				// 从闪电动画上方开始
				XMFLOAT3 pos = m_BoltAnim.GetTransform().GetPosition();
				XMFLOAT3 look{ 0.0f, 0.0f, 1.0f };
				XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
				pos.y += 3;
				cam1st->LookTo(pos, look, up);

				m_CameraMode = CameraMode::Free;
			}
		}
	}
	ImGui::End();
	ImGui::Render();

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());

	static int currBoltFrame = 0;
	static float frameTime = 0.0f;
	m_BoltAnim.SetTexture(m_BoltSRVs[currBoltFrame].Get());
	if (frameTime > 1.0f / 30)
	{
		currBoltFrame = (currBoltFrame + 1) % 60;
		frameTime -= 1.0f / 30;
	}
	frameTime += dt;
	*/

	if (m_ShowMode == Mode::SplitedTriangle)
	{
		m_BasicEffect.SetWorldMatrix(XMMatrixIdentity());
	}
	else
	{
		static float phi = 0.0f, theta = 0.0f;
		phi += 0.2f * dt, theta += 0.3f * dt;
		m_BasicEffect.SetWorldMatrix(XMMatrixRotationX(phi) * XMMatrixRotationY(theta));
	}

	if (ImGui::Begin("Geometry Shader Beginning"))
	{
		static int curr_item = 0;
		static const char* modes[] =
		{
			"Splited Triangle",
			"Cylinder w/o Cap"
		};
		if (ImGui::Combo("Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			m_ShowMode = static_cast<Mode>(curr_item);
			if (curr_item == 0)
			{
				ResetTriangle();
				m_BasicEffect.SetRenderSplitedTriangle(m_pd3dImmediateContext.Get());
			}
			else
			{
				ResetRoundWire();
				m_BasicEffect.SetRenderCylindeNoCap(m_pd3dImmediateContext.Get());
			}
			UINT stride = curr_item ? sizeof(VertexPosNormalColor) : sizeof(VertexPosColor);
			UINT offset = 0;
			m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
		}
		if (curr_item == 1)
		{
			static bool show_normal = false;
			if (ImGui::Checkbox("Show Normal", &show_normal))
			{
				m_ShowMode = show_normal ? Mode::CylinderNoCapWithNormal : Mode::CylinderNoCap;
			}
		}
	}
	ImGui::End();
	ImGui::Render();
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_BasicEffect.Apply(m_pd3dImmediateContext.Get());
	m_pd3dImmediateContext->Draw(m_VertexCount, 0);

	if (m_ShowMode == Mode::CylinderNoCapWithNormal)
	{
		m_BasicEffect.SetRenderNormal(m_pd3dImmediateContext.Get());
		m_BasicEffect.Apply(m_pd3dImmediateContext.Get());

		m_pd3dImmediateContext->Draw(m_VertexCount, 0);
		m_BasicEffect.SetRenderCylindeNoCap(m_pd3dImmediateContext.Get());
	}

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, 0));

	/*

	// ******************
	// 1. 给镜面反射区域写入值1到模板缓冲区
	//

	m_BasicEffect.SetWriteStencilOnly(m_pd3dImmediateContext.Get(), 1);
	m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 2. 绘制不透明的反射物体
	//

	// 开启反射绘制
	m_BasicEffect.SetReflectionState(true);
	m_BasicEffect.SetRenderDefaultWithStencil(m_pd3dImmediateContext.Get(), 1);

	m_Walls[2].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[3].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[4].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 3. 绘制不透明反射物体的阴影
	//

	m_WoodCrate.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射开启，阴影开启
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 1);

	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 恢复到原来的状态
	m_BasicEffect.SetShadowState(false);
	m_WoodCrate.SetMaterial(m_WoodCrateMat);

	// ******************
	// 4. 绘制透明镜面
	//
	m_BasicEffect.SetDrawBoltAnimNoDepthWriteWithStencil(m_pd3dImmediateContext.Get(), 1);
	m_BoltAnim.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 关闭反射绘制
	m_BasicEffect.SetReflectionState(false);
	m_BasicEffect.SetRenderAlphaBlendWithStencil(m_pd3dImmediateContext.Get(), 1);
	m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 5. 绘制不透明的正常物体
	//
	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());

	for (auto& wall : m_Walls)
		wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 6. 绘制不透明正常物体的阴影
	//
	m_WoodCrate.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射关闭，阴影开启
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);

	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_WoodCrate.SetMaterial(m_WoodCrateMat);

	//
	//m_BasicEffect.SetDrawBoltAnimNoDepthWrite(m_pd3dImmediateContext.Get());

	m_BasicEffect.SetDrawBoltAnimNoDepthTest(m_pd3dImmediateContext.Get());
	m_BoltAnim.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, 0));

	*/
}



bool GameApp::InitResource()
{
	//默认绘制三角形
	ResetTriangle();

	// ******************
	// 初始化不会变化的值
	//

	// 环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	// 
	Material material{};
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	m_BasicEffect.SetMaterial(material);

	//
	m_BasicEffect.SetEyePos(XMFLOAT3(0.0f, 0.0f, -0.5f));
	//
	m_BasicEffect.SetWorldMatrix(XMMatrixIdentity());
	m_BasicEffect.SetViewMatrix(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f),
		XMVectorZero(),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
	m_BasicEffect.SetCylinderHeight(2.0f);

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosColor);		// 跨越字节数
	UINT offset = 0;							// 起始偏移量
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_BasicEffect.SetRenderSplitedTriangle(m_pd3dImmediateContext.Get());

	/*

	// ******************
	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	m_WoodCrateMat = material;
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	m_BoltSRVs.assign(60, nullptr);
	wchar_t wstr[50];
	// 初始化闪电
	for (int i = 1; i <= 60; ++i)
	{
		wsprintf(wstr, L".\\Texture\\BoltAnim\\Bolt%03d.bmp", i);
		HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), wstr, nullptr, m_BoltSRVs[static_cast<size_t>(i) - 1].GetAddressOf()));
	}
	m_BoltAnim.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateCylinderNoCap(4.0f, 4.0f));
	m_BoltAnim.GetTransform().SetPosition(0.0f, 2.01f, 0.0f);
	m_BoltAnim.SetMaterial(material);

	// 初始化木盒
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L".\\Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox());
	// 抬起高度避免深度缓冲区资源争夺
	m_WoodCrate.GetTransform().SetPosition(0.0f, 0.01f, 0.0f);
	m_WoodCrate.SetTexture(texture.Get());
	m_WoodCrate.SetMaterial(material);

	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L".\\Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(),
		Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	// 初始化墙体
	m_Walls.resize(5);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L".\\Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// 这里控制墙体五个面的生成，0和1的中间位置用于放置镜面
	//     ____     ____
	//    /| 0 |   | 1 |\
	//   /4|___|___|___|2\
	//  /_/_ _ _ _ _ _ _\_\
	// | /       3       \ |
	// |/_________________\|
	//
	for (int i = 0; i < 5; ++i)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
	}
	m_Walls[0].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[1].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[2].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[3].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[4].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));

	m_Walls[0].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[0].GetTransform().SetPosition(-7.0f, 3.0f, 10.0f);
	m_Walls[1].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[1].GetTransform().SetPosition(7.0f, 3.0f, 10.0f);
	m_Walls[2].GetTransform().SetRotation(-XM_PIDIV2, XM_PIDIV2, 0.0f);
	m_Walls[2].GetTransform().SetPosition(10.0f, 3.0f, 0.0f);
	m_Walls[3].GetTransform().SetRotation(-XM_PIDIV2, XM_PI, 0.0f);
	m_Walls[3].GetTransform().SetPosition(0.0f, 3.0f, -10.0f);
	m_Walls[4].GetTransform().SetRotation(-XM_PIDIV2, -XM_PIDIV2, 0.0f);
	m_Walls[4].GetTransform().SetPosition(-10.0f, 3.0f, 0.0f);

	// 初始化镜面
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L".\\Texture\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Mirror.SetBuffer(m_pd3dDevice.Get(),
		Geometry::CreatePlane(XMFLOAT2(8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f)));
	m_Mirror.GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Mirror.GetTransform().SetPosition(0.0f, 3.0f, 10.0f);
	m_Mirror.SetTexture(texture.Get());
	m_Mirror.SetMaterial(material);

	// ******************
	// 初始化摄像机
	//

	auto camera = std::make_shared<ThirdPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetTarget(m_BoltAnim.GetTransform().GetPosition());
	camera->SetDistance(5.0f);
	camera->SetDistanceMinMax(2.0f, 14.0f);
	camera->SetRotationX(XM_PIDIV2);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);

	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());

	// ******************
	// 初始化不会变化的值
	//

	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

	// 环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	// 灯光
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, -10.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 25.0f;
	m_BasicEffect.SetPointLight(0, pointLight);

	// ******************
	// 设置调试对象名
	//
	m_Floor.SetDebugObjectName("Floor");
	m_Mirror.SetDebugObjectName("Mirror");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	m_Walls[4].SetDebugObjectName("Walls[4]");
	m_WoodCrate.SetDebugObjectName("WoodCrate");
	m_BoltAnim.SetDebugObjectName("BoltAnim");

	*/

	return true;
}

void GameApp::ResetTriangle()
{
	// 初始化三角形

	// 设置三角形顶点
	VertexPosColor vertices[] =
	{
		{ XMFLOAT3(-1.0f * 3, -0.866f * 3, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f * 3, 0.866f * 3, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * 3, -0.866f * 3, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	m_VertexCount = 2;

	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "TriangleVertexBuffer");
}

void GameApp::ResetRoundWire()
{
	// 初始化圆线
	// 设置圆边上各顶点
	// 必须要按顺时针设置
	// 由于要形成闭环，起始点需要使用2次

	VertexPosNormalColor vertices[41];
	for (int i = 0; i < 40; ++i)
	{
		vertices[i].pos = XMFLOAT3(cosf(XM_PI / 20 * i), -1.0f, -sinf(XM_PI / 20 * i));
		vertices[i].normal = XMFLOAT3(cosf(XM_PI / 20 * i), 0.0f, -sinf(XM_PI / 20 * i));
		vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	vertices[40] = vertices[0];

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf()));

	m_VertexCount = 41;

	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "CylinderVertexBuffer");
}
