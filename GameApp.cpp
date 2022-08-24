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

	m_GpuTimer_PreZ.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer_Lighting .Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer_Geometry.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer_Skybox.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_ForwardEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_DeferredEffect.InitAll(m_pd3dDevice.Get()))
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

	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_ForwardEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_DeferredEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
		m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
	}
	ResizeBuffers(m_ClientWidth, m_ClientHeight, m_MsaaSamples);
}

void GameApp::UpdateScene(float dt)
{
	m_CameraController.Update(dt);

	bool need_gpu_timer_reset = false;
	if (ImGui::Begin("Deferred Rendering"))
	{
		static const char* msaa_modes[] = {
			"None",
			"2x MSAA",
			"4x MSAA",
			"8x MSAA"
		};
		static int curr_msaa_item = 0;
		if (ImGui::Combo("MSAA", &curr_msaa_item, msaa_modes, ARRAYSIZE(msaa_modes)))
		{
			switch (curr_msaa_item)
			{
			case 0: m_MsaaSamples = 1; break;
			case 1: m_MsaaSamples = 2; break;
			case 2: m_MsaaSamples = 4; break;
			case 3: m_MsaaSamples = 8; break;
			}
			ResizeBuffers(m_ClientWidth, m_ClientHeight, m_MsaaSamples);
			m_SkyboxEffect.SetMsaaSamples(m_MsaaSamples);
			m_DeferredEffect.SetMsaaSamples(m_MsaaSamples);
			need_gpu_timer_reset = true;
		}
		static const char* light_culliing_modes[] = {
		   "Forward No Culling",
		   "Forward Pre-Z No Culling",
		   "Deferred No Culling"
		};
		static int curr_light_culliing_item = static_cast<int>(m_LightCullTechnique);
		if (ImGui::Combo("Light Culling", &curr_light_culliing_item, light_culliing_modes, ARRAYSIZE(light_culliing_modes)))
		{
			m_LightCullTechnique = static_cast<LightCullTechnique>(curr_light_culliing_item);
			need_gpu_timer_reset = true;
		}
		if (ImGui::Checkbox("Animate Lights", &m_AnimateLights))
			need_gpu_timer_reset = true;
		if (m_AnimateLights)
		{
			UpdateLights(dt);
		}

		if (ImGui::Checkbox("Lighting Only", &m_LightingOnly))
		{
			m_ForwardEffect.SetLightingOnly(m_LightingOnly);
			m_DeferredEffect.SetLightingOnly(m_LightingOnly);
			need_gpu_timer_reset = true;
		}
		if (ImGui::Checkbox("Face Normals", &m_FaceNormals))
		{
			m_ForwardEffect.SetFaceNormals(m_FaceNormals);
			m_DeferredEffect.SetFaceNormals(m_FaceNormals);
			need_gpu_timer_reset = true;
		}

		if (ImGui::Checkbox("Visualize Light Count", &m_VisualizeLightCount))
		{
			m_ForwardEffect.SetVisualizeLightCount(m_VisualizeLightCount);
			m_DeferredEffect.SetVisualizeLightCount(m_VisualizeLightCount);
			need_gpu_timer_reset = true;
		}

		if (m_LightCullTechnique >= LightCullTechnique::CULL_DEFERRED_NONE)
		{
			ImGui::Checkbox("Clear G-Buffers", &m_ClearGBuffers);
			if (m_MsaaSamples > 1 && ImGui::Checkbox("Visualize Shading Freq", &m_VisualizeShadingFreq)) 
			{
				m_DeferredEffect.SetVisualizeShadingFreq(m_VisualizeShadingFreq);
				need_gpu_timer_reset = true;
			}
		}
		ImGui::Text("Light Height Scale");
		ImGui::PushID(0);
		if (ImGui::SliderFloat("", &m_LightHeightScale, 0.25f, 1.0f))
		{
			UpdateLights(0.0f);
		}
		ImGui::PopID();

		static int light_level = static_cast<int>(log2f(static_cast<float>(m_ActiveLights)));
		ImGui::Text("Lights: %d", m_ActiveLights);
		ImGui::PushID(1);
		if (ImGui::SliderInt("", &light_level, 0, 10, ""))
		{
			m_ActiveLights = (1 << light_level);
			ResizeLights(m_ActiveLights);
			UpdateLights(0.0f);
			need_gpu_timer_reset = true;
		}
		ImGui::PopID();
	}
	ImGui::End();

	if (need_gpu_timer_reset) 
	{
		m_GpuTimer_PreZ.Reset(m_pd3dImmediateContext.Get());
		m_GpuTimer_Lighting.Reset(m_pd3dImmediateContext.Get());
		m_GpuTimer_Geometry.Reset(m_pd3dImmediateContext.Get());
		m_GpuTimer_Skybox.Reset(m_pd3dImmediateContext.Get());
	}

	m_ForwardEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	m_DeferredEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
	SetupLights(m_pCamera->GetViewMatrixXM());

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, m_pCamera->GetProjMatrixXM());
	frustum.Transform(frustum, m_pCamera->GetLocalToWorldMatrixXM());
	m_Sponza.FrustumCulling(frustum);
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

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

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

void GameApp::InitLightParams()
{
}

DirectX::XMFLOAT3 GameApp::HueToRGB(float hue)
{
	return DirectX::XMFLOAT3();
}

void GameApp::ResizeLights(UINT activeLights)
{
}

void GameApp::UpdateLights(float dt)
{
}

void XM_CALLCONV GameApp::SetupLights(DirectX::XMMATRIX viewMatrix)
{
	XMVector3TransformCoordStream(&m_PointLightParams[0].posV, sizeof(PointLight),
		&m_PointLightPosWorlds[0], sizeof(XMFLOAT3), m_ActiveLights, viewMatrix);
	PointLight* pData = m_pLightBuffer->MapDiscard(m_pd3dImmediateContext.Get());
	memcpy_s(pData, sizeof(PointLight) * m_ActiveLights,
		m_PointLightParams.data(), sizeof(PointLight) * m_ActiveLights);
	m_pLightBuffer->Unmap(m_pd3dImmediateContext.Get());
}

void GameApp::ResizeBuffers(UINT width, UINT height, UINT msaaSamples)
{
	// 初始化延迟渲染所需资源 
	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = msaaSamples;
	sampleDesc.Quality = 0;
	m_pLitBuffer = std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height,
		DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	m_pDepthBuffer = std::make_unique<Depth2DMS>(m_pd3dDevice.Get(), width, height, sampleDesc,
		m_MsaaSamples > 1 ? DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits : DepthStencilBitsFlag::Depth_32Bits,
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	
	// 创建只读深度/模版视图 
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		m_pDepthBuffer->GetDepthStencil()->GetDesc(&desc);
		desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
		m_pd3dDevice->CreateDepthStencilView(m_pDepthBuffer->GetTexture(), &desc, m_pDepthBufferReadOnlyDSV.ReleaseAndGetAddressOf());
	}
	// G-Buffer
	// MRT要求所有的G-Buffer使用相同的MSAA采样等级
	m_pGBuffer.clear();
	// normal_specular
	m_pGBuffer.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));
	// albedo
	m_pGBuffer.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));
	// posZgrad
	m_pGBuffer.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R16G16_FLOAT, sampleDesc, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));

	// 设置GBuffer资源列表
	m_pGBufferRTVs.resize(m_pGBuffer.size(), 0);
	m_pGBufferSRVs.resize(4, 0);
	for (std::size_t i = 0; i < m_pGBuffer.size(); ++i) 
	{
		m_pGBufferRTVs[i] = m_pGBuffer[i]->GetRenderTarget();
		m_pGBufferSRVs[i] = m_pGBuffer[i]->GetShaderResource();
	}

	// 深度缓冲区作为最后的SRV用于读取
	m_pGBufferSRVs.back() = m_pDepthBuffer->GetShaderResource();

	// 调试用缓冲区
	m_pDebugNormalGBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDebugAlbedoGBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pDebugPosZGradGBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R16G16_FLOAT);

	// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(m_pDepthBufferReadOnlyDSV.Get(), "DepthBufferReadOnlyDSV");
	m_pDepthBuffer->SetDebugObjectName("DepthBuffer");
	m_pLitBuffer->SetDebugObjectName("LitBuffer");
	m_pGBuffer[0]->SetDebugObjectName("GBuffer_Normal_Specular");
	m_pGBuffer[1]->SetDebugObjectName("GBuffer_Albedo");
	m_pGBuffer[2]->SetDebugObjectName("GBuffer_PosZgrad");
#endif
}

void GameApp::RenderForward(bool doPreZ)
{
}

void GameApp::RenderGBuffer()
{
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