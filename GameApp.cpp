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

	//m_TextureManager.Init(m_pd3dDevice.Get());
	//m_ModelManager.Init(m_pd3dDevice.Get());
	//
	//// 务必先初始化所有渲染状态，以供下面的特效使用
	//RenderStates::InitAll(m_pd3dDevice.Get());
	//
	//if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;
	//
	//if (!m_SkyBoxEffect.InitAll(m_pd3dDevice.Get()))
	//	return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	/*
	D3DApp::OnResize();

	m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
	m_pDepthTexture->SetDebugObjectName("DepthTexture");

	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_SkyBoxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
	*/
}

void GameApp::UpdateScene(float dt)
{
	/*
	m_CameraController.Update(dt);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

	if (ImGui::Begin("Normal Mapping"))
	{
		ImGui::Checkbox("Enable Normalmap", &m_EnableNormalMap);

		static const char* ground_strs[] = {
			"Floor",
			"Stones"
		};
		static int ground_item = static_cast<int>(m_GroundMode);
		if (ImGui::Combo("Ground Mode", &ground_item, ground_strs, ARRAYSIZE(ground_strs)))
		{
			Model* pModel = m_ModelManager.GetModel("Ground");
			switch (ground_item)
			{
			case 0:
				pModel->materials[0].Set<std::string>("$Diffuse", "floor");
				pModel->materials[0].Set<std::string>("$Normal", "floorN");
				break;
			case 1:
				pModel->materials[0].Set<std::string>("$Diffuse", "stones");
				pModel->materials[0].Set<std::string>("$Normal", "stonesN");
				break;
			default:
				break;
			}
		}
	}
	ImGui::End();
	ImGui::Render();

	m_SphereRad += dt * 2.0f;
	for (size_t i = 0; i < 4; ++i)
	{
		auto& transform = m_Spheres[i].GetTransform();
		auto pos = transform.GetPosition();
		pos.y = 0.5f * std::sin(m_SphereRad);
		transform.SetPosition(pos);
	}
	m_Spheres[4].GetTransform().RotateAround(XMFLOAT3(), XMFLOAT3(0.0f, 1.0f, 0.0f), 2.0f * dt);
	*/
}

void GameApp::DrawScene()
{
	/*
	// 创建后备缓冲区的渲染目标视图
	if (m_FrameCount < m_BackBufferCount)
	{
		ComPtr<ID3D11Texture2D> pBackBuffer;
		m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
		CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
	}

	// 绘制场景

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

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, m_pCamera->GetProjMatrixXM());
	frustum.Transform(frustum, m_pCamera->GetLocalToWorldMatrixXM());

	m_CenterSphere.FrustumCulling(frustum);
	m_BasicEffect.SetEyePos(m_CenterSphere.GetTransform().GetPosition());
	if (m_CenterSphere.InFrustum())
	{
		for (int i = 0; i < 6; ++i)
		{
			m_pCubeCamera->LookTo(m_CenterSphere.GetTransform().GetPosition(), looks[i], ups[i]);
			DrawScene(false, *m_pCubeCamera, m_pDynamicTextureCube->GetRenderTarget(i), m_pDynamicCubeDepthTexture->GetDepthStencil());
		}
	}

	DrawScene(true, *m_pCamera, GetBackBufferRTV(), m_pDepthTexture->GetDepthStencil());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
	*/
	ImGui::Begin("Normal Mapping");
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

void GameApp::Compute()
{
	assert(m_pd3dImmediateContext);

	// GPU 排序
	m_Timer.Reset();
	m_Timer.Start();
	m_GpuTimer.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer.Start();
	GPUSort();
	m_GpuTimer.Stop();
	double gpuComputeTime = m_GpuTimer.GetTime();

	// 结果回读到CPU进行比较
	m_pd3dImmediateContext->CopyResource(m_pTypedBufferCopy.Get(), m_pTypedBuffer1.Get());
	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_pd3dImmediateContext->Map(m_pTypedBufferCopy.Get(), 0, D3D11_MAP_READ, 0, &mappedData);
	m_Timer.Tick();
	m_Timer.Stop();
	float gpuTotalTime = m_Timer.TotalTime();

	// CPU 排序
	m_Timer.Reset();
	m_Timer.Start();
	std::sort(m_RandomNums.begin(), m_RandomNums.begin() + m_RandomNumsCount);
	m_Timer.Tick();
	m_Timer.Stop();

	float cpuTotalTime = m_Timer.TotalTime();

	bool isSame = !memcmp(mappedData.pData, m_RandomNums.data(),
		sizeof(uint32_t) * m_RandomNums.size());

	m_pd3dImmediateContext->Unmap(m_pTypedBufferCopy.Get(), 0);
	std::wstring wstr = L"排序元素数目：" + std::to_wstring(m_RandomNumsCount) +
		L"/" + std::to_wstring(m_RandomNums.size());
	wstr += L"\nGPU计算用时：" + std::to_wstring(gpuComputeTime) + L"秒";
	wstr += L"\nGPU总用时：" + std::to_wstring(gpuTotalTime) + L"秒";
	wstr += L"\nCPU用时：" + std::to_wstring(cpuTotalTime) + L"秒";
	wstr += isSame ? L"\n排序结果一致" : L"\n排序结果不一致";
	MessageBox(nullptr, wstr.c_str(), L"排序结束", MB_OK);
}

bool GameApp::InitResource()
{
	/*
	ComPtr<ID3D11Texture2D> pTex;
	D3D11_TEXTURE2D_DESC texDesc;
	std::string filenameStr;
	std::vector<ID3D11ShaderResourceView*> pCubeTextures;
	std::unique_ptr<TextureCube> pTexCube;
	{
		filenameStr = "Texture\\daylight0.png";
		for (size_t i = 0; i < 6; ++i)
		{
			filenameStr[16] = '0' + (char)i;
			pCubeTextures.push_back(m_TextureManager.CreateTexture(filenameStr, false, true));
		}
		pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
		pTex->GetDesc(&texDesc);
		pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		pTexCube->SetDebugObjectName("Daylight");
		for (uint32_t i = 0; i < 6; ++i)
		{
			pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
			m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
		}
		m_TextureManager.AddTexture("Daylight", pTexCube->GetShaderResource());
	}

	m_pDynamicTextureCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDynamicCubeDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), 256, 256);
	m_TextureManager.AddTexture("DynamicCube", m_pDynamicTextureCube->GetShaderResource());

	m_pDynamicTextureCube->SetDebugObjectName("DynamicTextureCube");
	m_pDynamicCubeDepthTexture->SetDebugObjectName("DynamicCubeDepthTexture");

	// 初始化游戏对象
	// 球体
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Sphere", Geometry::CreateSphere());
		pModel->SetDebugObjectName("Sphere");
		m_TextureManager.CreateTexture("Texture\\stone.dds", false, true);
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\stone.dds");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

		Transform sphereTransforms[] = {
		Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, 4.5f)),
		Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, 4.5f)),
		Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, -4.5f)),
		Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, -4.5f)),
		Transform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(2.5f, 0.0f, 0.0f)),
		};

		for (size_t i = 0; i < 5; ++i)
		{
			m_Spheres[i].GetTransform() = sphereTransforms[i];
			m_Spheres[i].SetModel(pModel);
		}
		m_CenterSphere.SetModel(pModel);
	}
	// 地面
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreatePlane(XMFLOAT2(10.0f, 10.0f), XMFLOAT2(5.0f, 5.0f)));
		pModel->SetDebugObjectName("Ground");
		m_TextureManager.AddTexture("floor", m_TextureManager.CreateTexture("Texture\\floor.dds"));
		m_TextureManager.AddTexture("floorN", m_TextureManager.CreateTexture("Texture\\floor_nmap.dds"));
		m_TextureManager.AddTexture("stones", m_TextureManager.CreateTexture("Texture\\stones.dds"));
		m_TextureManager.AddTexture("stonesN", m_TextureManager.CreateTexture("Texture\\stones_nmap.dds"));
		pModel->materials[0].Set<std::string>("$Diffuse", "floor");
		pModel->materials[0].Set<std::string>("$Normal", "floorN");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
		m_Ground.SetModel(pModel);
		m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);
	}
	// 柱体
	{
		Model* pModel = m_ModelManager.CreateFromGeometry("Cylinder", Geometry::CreateCylinder(0.5f, 2.0f));
		pModel->SetDebugObjectName("Cylinder");
		m_TextureManager.CreateTexture("Texture\\bricks.dds");
		m_TextureManager.CreateTexture("Texture\\bricks_nmap.dds");
		pModel->materials[0].Set<std::string>("$Diffuse", "Texture\\bricks.dds");
		pModel->materials[0].Set<std::string>("$Normal", "Texture\\bricks_nmap.dds");
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
			m_Cylinders[i].SetModel(pModel);
			m_Cylinders[i].GetTransform() = cylinderTransforms[i];
		}
	}

	Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
	pModel->SetDebugObjectName("Skybox");
	pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
	m_Skybox.SetModel(pModel);

	// ******************
	// 初始化摄像机
	//

	//auto camera = std::make_shared<ThirdPersonCamera>();
	//m_pCamera = camera;
	//camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	//camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	//camera->SetDistance(15.0f);
	//camera->SetDistanceMinMax(6.0f, 100.0f);
	//camera->SetRotationX(XM_PIDIV4);
	//camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

	m_pCamera = std::make_shared<FirstPersonCamera>();
	m_CameraController.InitCamera(m_pCamera.get());
	m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	m_pCamera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pCubeCamera = std::make_shared<FirstPersonCamera>();
	m_pCubeCamera->SetFrustum(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);
	m_pCubeCamera->SetViewPort(0.0f, 0.0f, 256.0f, 256.0f);

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
	*/

	// 初始化随机数数据
	std::mt19937 randEngine;
	randEngine.seed(std::random_device()());
	std::uniform_int_distribution<uint32_t> powRange(9.18);
	// 元素数目必须为2的次幂且不小于512个，并用最大值填充
	uint32_t elemCount = 1 << 18;
	m_RandomNums.assign(elemCount, UINT_MAX);
	// 填充随机数目的随机数，数目在一半容量到最大容量之间
	std::uniform_int_distribution<uint32_t> numsCountRange((uint32_t)m_RandomNums.size() / 2, (uint32_t)m_RandomNums.size());
	m_RandomNumsCount = elemCount;
	std::generate(m_RandomNums.begin(), m_RandomNums.begin() + m_RandomNumsCount, [&] {return randEngine(); });

	CD3D11_BUFFER_DESC bufferDesc(
		(uint32_t)m_RandomNums.size() * sizeof(uint32_t),
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_RandomNums.data();
	m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, m_pTypedBuffer1.GetAddressOf());
	m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, m_pTypedBuffer2.GetAddressOf());

	bufferDesc.BindFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, m_pTypedBufferCopy.GetAddressOf());

	bufferDesc = CD3D11_BUFFER_DESC(sizeof(CB), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	m_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, m_pConstantBuffer.GetAddressOf());

	// 创建着色器资源视图
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_BUFFER, DXGI_FORMAT_R32_UINT, 0, (uint32_t)m_RandomNums.size());
	m_pd3dDevice->CreateShaderResourceView(m_pTypedBuffer1.Get(), &srvDesc, m_pDataSRV1.GetAddressOf());
	m_pd3dDevice->CreateShaderResourceView(m_pTypedBuffer2.Get(), &srvDesc, m_pDataSRV2.GetAddressOf());

	// 创建无需访问视图
	CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = (UINT)m_RandomNums.size();
	m_pd3dDevice->CreateUnorderedAccessView(m_pTypedBuffer1.Get(), &uavDesc, m_pDataUAV1.GetAddressOf());
	m_pd3dDevice->CreateUnorderedAccessView(m_pTypedBuffer2.Get(), &uavDesc, m_pDataUAV2.GetAddressOf());

	// 创建计算着色器
	ComPtr<ID3DBlob> blob;
	D3DReadFileToBlob(L"HLSL\\BitonicSort_CS.cso", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pBitonicSort_CS.GetAddressOf());
	D3DReadFileToBlob(L"HLSL\\MatrixTranspose_CS.cso", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pMatrixTranspose_CS.GetAddressOf());

	return true;
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

void GameApp::SetConstants(UINT level, UINT descendMask, UINT matrixWidth, UINT matrixHeight)
{
	D3D11_MAPPED_SUBRESOURCE mappedData{};
	m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	CB cb = { level,descendMask,matrixWidth,matrixHeight };
	memcpy_s(mappedData.pData, sizeof(cb), &cb, sizeof(cb));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
	m_pd3dImmediateContext->CSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
}

void GameApp::GPUSort()
{
	UINT size = (UINT)m_RandomNums.size();

	m_pd3dImmediateContext->CSSetShader(m_pBitonicSort_CS.Get(), nullptr, 0);
	m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, m_pDataUAV1.GetAddressOf(), nullptr);

	// 按行数据进行排序，先排序level <= BLOCK_SIZE 的所有情况
	for (UINT level = 2; level <= size && level <= BITONIC_BLOCK_SIZE; level *= 2)
	{
		SetConstants(level, level, 0, 0);
		m_pd3dImmediateContext->Dispatch((size + BITONIC_BLOCK_SIZE - 1) / BITONIC_BLOCK_SIZE, 1, 1);
	}

	// 计算相近的矩阵宽高(宽>=高且需要都为2的次幂)
	UINT matrixWidth = 2, matrixHeight = 2;
	while (matrixWidth * matrixWidth < size)
	{
		matrixWidth *= 2;
	}
	matrixHeight = size / matrixWidth;

	// 排序level > BLOCK_SIZE 的所有情况
	ComPtr<ID3D11ShaderResourceView> pNullSRV;
	for (UINT level = BITONIC_BLOCK_SIZE * 2; level <= size; level *= 2)
	{
		if (level == size)
		{
			SetConstants(level / matrixWidth, level, matrixWidth, matrixHeight);
		}
		else
		{
			SetConstants(level / matrixWidth, level / matrixWidth, matrixWidth, matrixHeight);
		}
		// 先进行转置，并把数据输出到Buffer2
		m_pd3dImmediateContext->CSSetShader(m_pMatrixTranspose_CS.Get(), nullptr, 0);
		m_pd3dImmediateContext->CSSetShaderResources(0, 1, pNullSRV.GetAddressOf());
		m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, m_pDataUAV2.GetAddressOf(), nullptr);
		m_pd3dImmediateContext->CSSetShaderResources(0, 1, m_pDataSRV1.GetAddressOf());
		m_pd3dImmediateContext->Dispatch(matrixWidth / TRANSPOSE_BLOCK_SIZE, matrixHeight / TRANSPOSE_BLOCK_SIZE, 1);

		// 对Buffer2 进行排序
		m_pd3dImmediateContext->CSSetShader(m_pBitonicSort_CS.Get(), nullptr, 0);
		m_pd3dImmediateContext->Dispatch(size / BITONIC_BLOCK_SIZE, 1, 1);

		// 接着转置回来，并把数据输出到Buffer1
		SetConstants(matrixWidth, level, matrixWidth, matrixHeight);
		m_pd3dImmediateContext->CSSetShader(m_pMatrixTranspose_CS.Get(), nullptr, 0);
		m_pd3dImmediateContext->CSSetShaderResources(0, 1, pNullSRV.GetAddressOf());
		m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, m_pDataUAV1.GetAddressOf(), nullptr);
		m_pd3dImmediateContext->CSSetShaderResources(0, 1, m_pDataSRV2.GetAddressOf());
		m_pd3dImmediateContext->Dispatch(matrixWidth / TRANSPOSE_BLOCK_SIZE, matrixHeight / TRANSPOSE_BLOCK_SIZE, 1);

		// 对Buffer1排序剩余行数据
		m_pd3dImmediateContext->CSSetShader(m_pBitonicSort_CS.Get(), nullptr, 0);
		m_pd3dImmediateContext->Dispatch(size / BITONIC_BLOCK_SIZE, 1, 1);

	}

}
