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

	m_GpuTimer_Lighting.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer_Shadow.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
	m_GpuTimer_Skybox.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_ForwardEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_ShadowEffect.InitAll(m_pd3dDevice.Get()))
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

	DXGI_SAMPLE_DESC sampleDesc{};
	sampleDesc.Count = m_MsaaSamples;
	sampleDesc.Quality = 0;
	m_pLitBuffer = std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	m_pDepthBuffer = std::make_unique<Depth2DMS>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, sampleDesc, DepthStencilBitsFlag::Depth_32Bits);

	if (m_pViewerCamera != nullptr)
	{
		m_pViewerCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 300.0f);
		m_pViewerCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);

		m_ForwardEffect.SetProjMatrix(m_pViewerCamera->GetProjMatrixXM(true));
	}
}

void GameApp::UpdateScene(float dt)
{
	if (m_CSManager.m_SelectedCamera <= CameraSelection::CameraSelection_Light)
	{
		m_FPSCameraController.Update(dt);
	}
#pragma region IMGUI
	bool need_gpu_timer_reset = false;
	if (ImGui::Begin("Cascaded Shadow Mapping"))
	{
		ImGui::Checkbox("Debug Shadow", &m_DebugShadow);

		static bool visualizeCascades = false;
		if (ImGui::Checkbox("Visualize Cascades", &visualizeCascades))
		{
			m_ForwardEffect.SetCascadeVisulization(visualizeCascades);
			need_gpu_timer_reset = true;
		}
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
			DXGI_SAMPLE_DESC sampleDesc;
			sampleDesc.Count = m_MsaaSamples;
			sampleDesc.Quality = 0;
			m_pLitBuffer = std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
			m_pDepthBuffer = std::make_unique<Depth2DMS>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, sampleDesc, DepthStencilBitsFlag::Depth_32Bits);
			m_SkyboxEffect.SetMsaaSamples(m_MsaaSamples);
			need_gpu_timer_reset = true;
		}
		static int texture_level = 10;
		ImGui::Text("Texture Size: %d", m_CSManager.m_ShadowSize);
		if (ImGui::SliderInt("##0", &texture_level, 9, 13, ""))
		{
			m_CSManager.m_ShadowSize = (1 << texture_level);
			m_CSManager.InitResource(m_pd3dDevice.Get());
			m_pDebugShadowBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_CSManager.m_ShadowSize, m_CSManager.m_ShadowSize, DXGI_FORMAT_R8G8B8A8_UNORM);
			need_gpu_timer_reset = true;
		}

		static int pcf_kernel_level = 1;
		ImGui::Text("PCF Kernel Size: %d", m_CSManager.m_PCFKernelSize);
		if (ImGui::SliderInt("##1", &pcf_kernel_level, 0, 15, ""))
		{
			m_CSManager.m_PCFKernelSize = 2 * pcf_kernel_level + 1;
			m_ForwardEffect.SetPCFKernelSize(m_CSManager.m_PCFKernelSize);
			need_gpu_timer_reset = true;
		}

		ImGui::Text("Depth Offset");
		if (ImGui::SliderFloat("##2", &m_CSManager.m_PCFDepthOffset, 0.0f, 0.05f))
		{
			m_ForwardEffect.SetPCFDepthOffset(m_CSManager.m_PCFDepthOffset);
		}
		if (ImGui::Checkbox("Cascade Blur", &m_CSManager.m_BlendBetweenCascades))
		{
			m_ForwardEffect.SetCascadeBlendEnabled(m_CSManager.m_BlendBetweenCascades);
			need_gpu_timer_reset = true;
		}
		if (ImGui::SliderFloat("##3", &m_CSManager.m_BlendBetweenCascadesRange, 0.0f, 0.5f))
		{
			m_ForwardEffect.SetCascadeBlendArea(m_CSManager.m_BlendBetweenCascadesRange);
		}

		if (ImGui::Checkbox("DDX, DDY offset", &m_CSManager.m_DerivativeBasedOffset))
		{
			m_ForwardEffect.SetPCFDerivativesOffsetEnabled(m_CSManager.m_DerivativeBasedOffset);
			need_gpu_timer_reset = true;
		}
		if (ImGui::Checkbox("Fixed Size Frustum AABB", &m_CSManager.m_FixedSizeFrustumAABB))
			need_gpu_timer_reset = true;
		if (ImGui::Checkbox("Fit Light to Texels", &m_CSManager.m_MoveLightTexelSize))
			need_gpu_timer_reset = true;

		static const char* fit_projection_strs[] = {
			"Fit Projection To Cascade",
			"Fit Projection To Scene"
		};
		if (ImGui::Combo("##4", reinterpret_cast<int*>(&m_CSManager.m_SelectedCascadesFit), fit_projection_strs, ARRAYSIZE(fit_projection_strs)))
			need_gpu_timer_reset = true;

		static const char* camera_strs[] = {
			"Main Camera",
			"Light Camera",
			"Cascade Camera 1",
			"Cascade Camera 2",
			"Cascade Camera 3",
			"Cascade Camera 4",
			"Cascade Camera 5",
			"Cascade Camera 6",
			"Cascade Camera 7",
			"Cascade Camera 8"
		};
		static int camera_idx = 0;
		if (camera_idx > m_CSManager.m_CascadeLevels + 2)
			camera_idx = m_CSManager.m_CascadeLevels + 2;
		if (ImGui::Combo("##5", &camera_idx, camera_strs, m_CSManager.m_CascadeLevels + 2))
		{
			m_CSManager.m_SelectedCamera = static_cast<CameraSelection>(camera_idx);
			if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Eye)
			{
				m_FPSCameraController.InitCamera(static_cast<FirstPersonCamera*>(m_pViewerCamera.get()));
				m_FPSCameraController.SetMouseSpeed(10.0f);
			}
			else if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Light)
			{
				m_FPSCameraController.InitCamera(static_cast<FirstPersonCamera*>(m_pLightCamera.get()));
				m_FPSCameraController.SetMouseSpeed(50.0f);
			}
		}

		static const char* fit_near_far_strs[] =
		{
			"0:1 NearFar",
			"Cascade AABB NearFar",
			"Scene AABB NearFar",
			"Scene AABB Intersection NearFar"
		};

		if (ImGui::Combo("##6", reinterpret_cast<int*>(&m_CSManager.m_SelectedNearFarFit), fit_near_far_strs, ARRAYSIZE(fit_near_far_strs)))
			need_gpu_timer_reset = true;

		static const char* cascade_selection_strs[] =
		{
			"Map-based Selection",
			"Interval-based Selection"
		};
		if (ImGui::Combo("##7", reinterpret_cast<int*>(&m_CSManager.m_SelectedCascadeSelection), cascade_selection_strs, ARRAYSIZE(cascade_selection_strs)))
		{
			m_ForwardEffect.SetCascadeIntervalSelectionEnabled(static_cast<bool>(m_CSManager.m_SelectedCascadeSelection));
			need_gpu_timer_reset = true;
		}

		static const char* cascade_levels[] = {
			"1 Level",
			"2 Levels",
			"3 Levels",
			"4 Levels",
			"5 Levels",
			"6 Levels",
			"7 Levels",
			"8 Levels"
		};
		static int cascade_level_index = m_CSManager.m_CascadeLevels - 1;
		if (ImGui::Combo("Cascade", &cascade_level_index, cascade_levels, ARRAYSIZE(cascade_levels)))
		{
			m_CSManager.m_CascadeLevels = cascade_level_index + 1;
			m_CSManager.InitResource(m_pd3dDevice.Get());
			m_ForwardEffect.SetCascadeLevels(m_CSManager.m_CascadeLevels);
			need_gpu_timer_reset = true;
		}

		char level_str[] = "Level1";
		for (int i = 0; i < m_CSManager.m_CascadeLevels; ++i)
		{
			level_str[5] = '1' + i;
			ImGui::SliderFloat(level_str, m_CSManager.m_CascadePartitionsPercentage + i, 0.0f, 1.0f, "");
			ImGui::SameLine();
			ImGui::Text("%.1f%%", m_CSManager.m_CascadePartitionsPercentage[i] * 100);
			if (i && m_CSManager.m_CascadePartitionsPercentage[i] < m_CSManager.m_CascadePartitionsPercentage[i - 1])
				m_CSManager.m_CascadePartitionsPercentage[i] = m_CSManager.m_CascadePartitionsPercentage[i - 1];
			if (i < m_CSManager.m_CascadeLevels - 1 && m_CSManager.m_CascadePartitionsPercentage[i] > m_CSManager.m_CascadePartitionsPercentage[i + 1])
				m_CSManager.m_CascadePartitionsPercentage[i] = m_CSManager.m_CascadePartitionsPercentage[i + 1];
			if (m_CSManager.m_CascadePartitionsPercentage[i] > 1.0f)
				m_CSManager.m_CascadePartitionsPercentage[i] = 1.0f;
		}
	}
	ImGui::End();

	if (need_gpu_timer_reset)
	{
		m_GpuTimer_Lighting.Reset(m_pd3dImmediateContext.Get());
		m_GpuTimer_Shadow.Reset(m_pd3dImmediateContext.Get());
		m_GpuTimer_Skybox.Reset(m_pd3dImmediateContext.Get());
	}

#pragma endregion IMGUI

	if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Eye)
	{
		m_ForwardEffect.SetViewMatrix(m_pViewerCamera->GetViewMatrixXM());
		m_ForwardEffect.SetProjMatrix(m_pViewerCamera->GetProjMatrixXM(true));
		m_SkyboxEffect.SetViewMatrix(m_pViewerCamera->GetViewMatrixXM());
		m_SkyboxEffect.SetProjMatrix(m_pViewerCamera->GetProjMatrixXM(true));
	}
	else if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Light)
	{
		m_ForwardEffect.SetViewMatrix(m_pLightCamera->GetViewMatrixXM());
		m_ForwardEffect.SetProjMatrix(m_pLightCamera->GetProjMatrixXM(true));
		m_SkyboxEffect.SetViewMatrix(m_pLightCamera->GetViewMatrixXM());
		m_SkyboxEffect.SetProjMatrix(m_pLightCamera->GetProjMatrixXM(true));
	}
	else
	{
		XMMATRIX ShadowProjRZ = m_CSManager.GetShadowProjectionXM(
			static_cast<int>(m_CSManager.m_SelectedCamera) - 2);
		ShadowProjRZ.r[2] *= g_XMNegateZ.v;
		ShadowProjRZ.r[3] = XMVectorSetZ(ShadowProjRZ.r[3], 1.0f - XMVectorGetZ(ShadowProjRZ.r[3]));

		m_ForwardEffect.SetViewMatrix(m_pLightCamera->GetViewMatrixXM());
		m_ForwardEffect.SetProjMatrix(ShadowProjRZ);
		m_SkyboxEffect.SetViewMatrix(m_pLightCamera->GetViewMatrixXM());
		m_SkyboxEffect.SetProjMatrix(ShadowProjRZ);
	}
	m_ShadowEffect.SetViewMatrix(m_pLightCamera->GetViewMatrixXM());
	m_CSManager.UpdateFrame(*m_pViewerCamera, *m_pLightCamera, m_Powerplant.GetModel()->boundingbox);
	m_ForwardEffect.SetLightDir(m_pLightCamera->GetLookAxis());
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	// 创建后备缓冲区的渲染目标视图
	if (m_FrameCount < m_BackBufferCount)
	{
		ComPtr<ID3D11Texture2D> pBackBuffer;
		m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
		CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
	}

	//场景渲染
	RenderShadowForAllCascades();
	RenderForward();
	RenderSkybox();

	if (ImGui::Begin("Cascaded Shadow Mapping"))
	{
		ImGui::Separator();
		ImGui::Text("GPU Profile");
		double total_time = 0.0f;

		m_GpuTimer_Shadow.TryGetTime(nullptr);
		ImGui::Text("Shadow Pass: %.3f ms", m_GpuTimer_Shadow.AverageTime() * 1000);
		total_time += m_GpuTimer_Shadow.AverageTime();

		m_GpuTimer_Lighting.TryGetTime(nullptr);
		ImGui::Text("Lighting Pass: %.3f ms", m_GpuTimer_Lighting.AverageTime() * 1000);
		total_time += m_GpuTimer_Lighting.AverageTime();

		m_GpuTimer_Skybox.TryGetTime(nullptr);
		ImGui::Text("Skybox Pass: %.3f ms", m_GpuTimer_Skybox.AverageTime() * 1000);
		total_time += m_GpuTimer_Skybox.AverageTime();

		ImGui::Text("Total: %.3f ms", total_time * 1000);
	}
	ImGui::End();

	if (m_DebugShadow)
	{
		if (ImGui::Begin("Debug Shadow"))
		{
			static const char* cascade_level_strs[] = {
				"Level 1",
				"Level 2",
				"Level 3",
				"Level 4",
				"Level 5",
				"Level 6",
				"Level 7",
				"Level 8"
			};
			static int curr_level_idx = 0;
			ImGui::Combo("##1", &curr_level_idx, cascade_level_strs, m_CSManager.m_CascadeLevels);
			if (curr_level_idx >= m_CSManager.m_CascadeLevels)
				curr_level_idx = m_CSManager.m_CascadeLevels - 1;

			D3D11_VIEWPORT vp = m_CSManager.GetShadowViewport();
			m_ShadowEffect.RenderDepthToTexture(m_pd3dImmediateContext.Get(),
				m_CSManager.GetCascadeOutput(curr_level_idx),
				m_pDebugShadowBuffer->GetRenderTarget(),
				vp);
			ImVec2 winSize = ImGui::GetWindowSize();
			float smaller = (std::min)(winSize.x - 20, winSize.y - 60);
			ImGui::Image(m_pDebugShadowBuffer->GetShaderResource(), ImVec2(smaller, smaller));
		}
		ImGui::End();
	}

	D3D11_VIEWPORT vp = m_pViewerCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &vp);
	ImGui::Render();

	ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
	m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

}

/*
void GameApp::DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
{
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
}
*/

bool GameApp::InitResource()
{
	// ******************
	// 初始化摄像机
	//

	auto viewerCamera = std::make_shared<FirstPersonCamera>();
	m_pViewerCamera = viewerCamera;

	//camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	//camera->SetTarget(XMFLOAT3(0.0f, 2.5f, 0.0f));
	//camera->SetDistance(20.0f);
	//camera->SetDistanceMinMax(10.0f, 90.0f);
	//camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	//camera->SetRotationX(XM_PIDIV4);

	m_pViewerCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_pViewerCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 300.0f);
	viewerCamera->LookAt(XMFLOAT3(100.0f, 5.0f, 5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	auto lightCamera = std::make_shared<FirstPersonCamera>();
	m_pLightCamera = lightCamera;

	m_pLightCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	lightCamera->LookAt(XMFLOAT3(-320.0f, 300.0f, -220.3f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	lightCamera->SetFrustum(XM_PI / 3, 1.0f, 0.1f, 1000.0f);

	m_FPSCameraController.InitCamera(viewerCamera.get());
	m_FPSCameraController.SetMouseSpeed(10.0f);

	// 初始化

	m_ForwardEffect.SetViewMatrix(viewerCamera->GetViewMatrixXM());
	m_ForwardEffect.SetProjMatrix(viewerCamera->GetProjMatrixXM(true));
	m_ForwardEffect.SetPCFKernelSize(m_CSManager.m_PCFKernelSize);
	m_ForwardEffect.SetPCFDepthOffset(m_CSManager.m_PCFDepthOffset);
	m_ForwardEffect.SetShadowSize(m_CSManager.m_ShadowSize);
	m_ForwardEffect.SetCascadeBlendEnabled(m_CSManager.m_BlendBetweenCascades);
	m_ForwardEffect.SetCascadeBlendArea(m_CSManager.m_BlendBetweenCascadesRange);
	m_ForwardEffect.SetCascadeLevels(m_CSManager.m_CascadeLevels);
	m_ForwardEffect.SetCascadeIntervalSelectionEnabled(static_cast<bool>(m_CSManager.m_SelectedCascadeSelection));

	m_ForwardEffect.SetPCFDerivativesOffsetEnabled(m_CSManager.m_DerivativeBasedOffset);

	m_ShadowEffect.SetViewMatrix(lightCamera->GetViewMatrixXM());
	m_SkyboxEffect.SetMsaaSamples(1);

	// 初始化对象
	m_Powerplant.SetModel(m_ModelManager.CreateFromFile("Model\\powerplant\\powerplant.obj"));

	m_ModelManager.CreateFromGeometry("cube", Geometry::CreateBox());
	m_Cube.SetModel(m_ModelManager.GetModel("cube"));
	m_Cube.GetTransform().SetPosition(XMFLOAT3(48.0f, 1.0f, 0.0f));

	m_TextureManager.CreateFromFile("Texture\\Clouds.dds");
	m_ModelManager.CreateFromGeometry("skyboxCube", Geometry::CreateBox());
	Model* pModel = m_ModelManager.GetModel("skyboxCube");
	pModel->materials[0].Set<std::string>("$Skybox", "Texture\\Clouds.dds");
	m_Skybox.SetModel(pModel);

	// 初始化阴影
	m_CSManager.InitResource(m_pd3dDevice.Get());
	m_pDebugShadowBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_CSManager.m_ShadowSize, m_CSManager.m_ShadowSize, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_pLitBuffer->SetDebugObjectName("LitBuffer");

	return true;
}

/*
void GameApp::InitLightParams()
{
	m_PointLightInitDatas.resize(MAX_LIGHTS);
	m_PointLightParams.resize(MAX_LIGHTS);
	m_PointLightPosWorlds.resize(MAX_LIGHTS);

	std::mt19937 rng(1337);
	constexpr float maxRadius = 100.0f;
	constexpr float attenuationStartFactor = 0.8f;
	std::uniform_real<float> radiusNormDist(0.0f, 1.0f);
	std::uniform_real<float> angleDist(0.0f, 2.0f * XM_PI);
	std::uniform_real<float> heightDist(0.0f, 75.0f);
	std::uniform_real<float> animationSpeedDist(2.0f, 20.0f);
	std::uniform_int<int> animationDirection(0, 1);
	std::uniform_real<float> hueDist(0.0f, 1.0f);
	std::uniform_real<float> intensityDist(0.2f, 0.8f);
	std::uniform_real<float> attenuationDist(2.0f, 20.0f);

	for (unsigned int i = 0; i < MAX_LIGHTS; ++i)
	{
		PointLight& params = m_PointLightParams[i];
		PointLightInitData& data = m_PointLightInitDatas[i];

		data.radius = std::sqrt(radiusNormDist(rng)) * maxRadius;
		data.angle = angleDist(rng);
		data.height = heightDist(rng);
		//归一化
		data.animationSpeed = (animationDirection(rng) * 2 - 1) * animationSpeedDist(rng) / data.radius;

		// HSL->RGB
		params.color = HueToRGB(hueDist(rng));
		XMStoreFloat3(&params.color, XMLoadFloat3(&params.color) * intensityDist(rng));
		params.attenuationEnd = attenuationDist(rng);
		params.attenuationBegin = attenuationStartFactor * params.attenuationEnd;
	}

}

DirectX::XMFLOAT3 GameApp::HueToRGB(float hue)
{
	float intPart;
	float fracPart = std::modf(hue * 6.0f, &intPart);
	int region = static_cast<int>(intPart);
	switch (region)
	{
	case 0: return XMFLOAT3(1.0f, fracPart, 0.0f);
	case 1: return XMFLOAT3(1.0f - fracPart, 1.0f, 0.0f);
	case 2: return XMFLOAT3(0.0f, 1.0f, fracPart);
	case 3: return XMFLOAT3(0.0f, 1.0f - fracPart, 1.0f);
	case 4: return XMFLOAT3(fracPart, 0.0f, 1.0f);
	case 5: return XMFLOAT3(1.0f, 0.0f, 1.0f - fracPart);
	}

	return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void GameApp::ResizeLights(UINT activeLights)
{
	m_ActiveLights = activeLights;
	m_pLightBuffer = std::make_unique<StructuredBuffer<PointLight>>(m_pd3dDevice.Get(), activeLights, D3D11_BIND_SHADER_RESOURCE, false, true);
	m_pLightBuffer->SetDebugObjectName("LightBuffer");
}

void GameApp::UpdateLights(float dt)
{
	static float totalTime = 0.0f;
	totalTime += dt;
	for (UINT i = 0; i < m_ActiveLights; ++i)
	{
		const auto& data = m_PointLightInitDatas[i];
		float angle = data.angle + totalTime * data.animationSpeed;
		m_PointLightPosWorlds[i] = XMFLOAT3(
			data.radius * std::cos(angle),
			data.height * m_LightHeightScale,
			data.radius * std::sin(angle)
		);
	}
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
	// 初始化前向渲染所需资源
	UINT tileWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	UINT tileHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	m_pTileBuffer = std::make_unique<StructuredBuffer<TileInfo>>(m_pd3dDevice.Get(),
		width * height, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

	// 初始化延迟渲染所需资源
	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = msaaSamples;
	sampleDesc.Quality = 0;
	m_pLitBuffer = std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height,
		DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	m_pFlatLitBuffer = std::make_unique<StructuredBuffer<DirectX::XMUINT2>>(
		m_pd3dDevice.Get(), width * height * msaaSamples,
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
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
	m_pGBuffers.clear();
	// normal_specular
	m_pGBuffers.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
	// albedo
	m_pGBuffers.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
	// posZgrad
	m_pGBuffers.push_back(std::make_unique<Texture2DMS>(m_pd3dDevice.Get(), width, height, DXGI_FORMAT_R16G16_FLOAT, sampleDesc, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));

	// 设置GBuffer资源列表
	m_pGBufferRTVs.resize(m_pGBuffers.size(), 0);
	m_pGBufferSRVs.resize(4, 0);
	for (std::size_t i = 0; i < m_pGBuffers.size(); ++i)
	{
		m_pGBufferRTVs[i] = m_pGBuffers[i]->GetRenderTarget();
		m_pGBufferSRVs[i] = m_pGBuffers[i]->GetShaderResource();
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
	m_pFlatLitBuffer->SetDebugObjectName("FlatLitBuffer");
	m_pGBuffers[0]->SetDebugObjectName("GBuffer_Normal_Specular");
	m_pGBuffers[1]->SetDebugObjectName("GBuffer_Albedo");
	m_pGBuffers[2]->SetDebugObjectName("GBuffer_PosZgrad");
#endif
}

void GameApp::RenderForward(bool doPreZ)
{
	float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(m_pLitBuffer->GetRenderTarget(), black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

	D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	// PreZ Pass
	if (doPreZ)
	{
		m_GpuTimer_PreZ.Start();
		m_pd3dImmediateContext->OMSetRenderTargets(0, 0, m_pDepthBuffer->GetDepthStencil());
		m_ForwardEffect.SetRenderPreZPass();

		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_ForwardEffect);
		m_pd3dImmediateContext->OMSetRenderTargets(0, 0, nullptr);
		m_GpuTimer_PreZ.Stop();
	}
	// 光源裁剪阶段
	if (m_LightCullTechnique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
	{
		m_GpuTimer_LightCulling.Start();
		m_ForwardEffect.ComputeTiledLightCulling(m_pd3dImmediateContext.Get(),
			m_pTileBuffer->GetUnorderedAccess(),
			m_pLightBuffer->GetShaderResource(),
			m_pGBufferSRVs[3]);

		m_GpuTimer_LightCulling.Stop();
	}
	else if (m_LightCullTechnique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_2_5D)
	{
		m_GpuTimer_LightCulling.Start();
		m_ForwardEffect.Compute2Point5LightCulling(m_pd3dImmediateContext.Get(),
			m_pTileBuffer->GetUnorderedAccess(),
			m_pLightBuffer->GetShaderResource(),
			m_pGBufferSRVs[3]);

		m_GpuTimer_LightCulling.Stop();
	}

	// 正常绘制
	m_GpuTimer_Lighting.Start();
	{
		ID3D11RenderTargetView* pRTVs[1] = { m_pLitBuffer->GetRenderTarget() };
		m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthBuffer->GetDepthStencil());
		if (m_LightCullTechnique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE || m_LightCullTechnique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_2_5D)
		{
			m_ForwardEffect.SetTileBuffer(m_pTileBuffer->GetShaderResource());
			m_ForwardEffect.SetRenderWithTiledLightCulling();
		}
		else
			m_ForwardEffect.SetRenderDefault();
		m_ForwardEffect.SetLightBuffer(m_pLightBuffer->GetShaderResource());
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_ForwardEffect);

		m_ForwardEffect.SetTileBuffer(nullptr);
		m_ForwardEffect.Apply(m_pd3dImmediateContext.Get());
		m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	m_GpuTimer_Lighting.Stop();
}

void GameApp::RenderGBuffer()
{
	// 注意：我们实际上只需要清空深度缓冲区，因为我们用天空盒的采样来替代没有写入的像素(例如：远平面)
	// 	    这里我们使用深度缓冲区来重建位置且只有在视锥体内的位置会被着色
	// 注意：反转Z的缓冲区，远平面为0
	if (m_ClearGBuffers)
	{
		float black[4] = { 0.0f,0.0f,0.0f,1.0f };
		for (auto rtv : m_pGBufferRTVs)
		{
			m_pd3dImmediateContext->ClearRenderTargetView(rtv, black);
		}
		m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
	}
	D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
	m_pd3dImmediateContext->RSSetViewports(1, &viewport);

	m_GpuTimer_Geometry.Start();
	{
		m_DeferredEffect.SetRenderGBuffer();
		m_pd3dImmediateContext->OMSetRenderTargets(static_cast<UINT>(m_pGBuffers.size()), m_pGBufferRTVs.data(), m_pDepthBuffer->GetDepthStencil());
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_DeferredEffect);
		m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	m_GpuTimer_Geometry.Stop();
}
*/


void GameApp::RenderShadowForAllCascades()
{
	m_GpuTimer_Shadow.Start();
	{
		m_ShadowEffect.SetRenderDefault();
		D3D11_VIEWPORT vp = m_CSManager.GetShadowViewport();
		m_pd3dImmediateContext->RSSetViewports(1, &vp);

		for (size_t cascadeIdx = 0; cascadeIdx < m_CSManager.m_CascadeLevels; ++cascadeIdx)
		{
			ID3D11RenderTargetView* nullRTV = nullptr;
			ID3D11DepthStencilView* depthDSV = m_CSManager.GetCascadeDepthStencilView(cascadeIdx);
			m_pd3dImmediateContext->ClearDepthStencilView(depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
			m_pd3dImmediateContext->OMSetRenderTargets(1, &nullRTV, depthDSV);

			XMMATRIX shadowProj = m_CSManager.GetShadowProjectionXM(cascadeIdx);
			m_ShadowEffect.SetProjMatrix(shadowProj);

			// 更新物体与投影立方体的裁剪
			BoundingOrientedBox obb = m_CSManager.GetShadowOBB(cascadeIdx);
			obb.Transform(obb, m_pLightCamera->GetLocalToWorldMatrixXM());

			m_Powerplant.CubeCulling(obb);
			m_Powerplant.Draw(m_pd3dImmediateContext.Get(), m_ShadowEffect);

			m_Cube.CubeCulling(obb);
			m_Cube.Draw(m_pd3dImmediateContext.Get(), m_ShadowEffect);

		}
	}
	m_GpuTimer_Shadow.Stop();
}

void GameApp::RenderForward()
{
	m_GpuTimer_Lighting.Start();
	{
		float black[4] = { 0.0f,0.0f,0.0f,1.0f };
		m_pd3dImmediateContext->ClearRenderTargetView(m_pLitBuffer->GetRenderTarget(), black);
		m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

		D3D11_VIEWPORT viewport = m_pViewerCamera->GetViewPort();
		BoundingFrustum frustum;
		if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Eye)
		{
			BoundingFrustum::CreateFromMatrix(frustum, m_pViewerCamera->GetProjMatrixXM());
			frustum.Transform(frustum, m_pViewerCamera->GetLocalToWorldMatrixXM());
			m_Powerplant.FrustumCulling(frustum);
			m_Cube.FrustumCulling(frustum);
		}
		else if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Light)
		{
			BoundingFrustum::CreateFromMatrix(frustum, m_pLightCamera->GetProjMatrixXM());
			frustum.Transform(frustum, m_pLightCamera->GetLocalToWorldMatrixXM());
			m_Powerplant.FrustumCulling(frustum);
			m_Cube.FrustumCulling(frustum);
		}
		else
		{
			BoundingOrientedBox bbox = m_CSManager.GetShadowOBB(static_cast<int>(m_CSManager.m_SelectedCamera) - 2);
			bbox.Transform(bbox, m_pLightCamera->GetLocalToWorldMatrixXM());
			m_Powerplant.CubeCulling(bbox);
			m_Cube.CubeCulling(bbox);
			viewport.Width = (float)std::min(m_ClientHeight, m_ClientWidth);
			viewport.Height = viewport.Width;
		}
		m_pd3dImmediateContext->RSSetViewports(1, &viewport);

		ID3D11RenderTargetView* pRTVs[1] = { m_pLitBuffer->GetRenderTarget() };
		m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthBuffer->GetDepthStencil());
		m_ForwardEffect.SetCascadeFrustumsEyeSpaceDepths(m_CSManager.GetCascadePartitions());
		static XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
		XMFLOAT4 scales[8]{};
		XMFLOAT4 offsets[8]{};
		for (size_t cascadeIndex = 0; cascadeIndex < m_CSManager.m_CascadeLevels; ++cascadeIndex)
		{
			XMMATRIX ShadowTexture = m_CSManager.GetShadowProjectionXM(cascadeIndex) * T;
			scales[cascadeIndex].x = XMVectorGetX(ShadowTexture.r[0]);
			scales[cascadeIndex].y = XMVectorGetY(ShadowTexture.r[1]);
			scales[cascadeIndex].z = XMVectorGetZ(ShadowTexture.r[2]);
			scales[cascadeIndex].w = 1.0f;
			XMStoreFloat3((XMFLOAT3*)(offsets + cascadeIndex), ShadowTexture.r[3]);
		}
		m_ForwardEffect.SetCascadeOffsets(offsets);
		m_ForwardEffect.SetCascadeScales(scales);
		m_ForwardEffect.SetShadowViewMatrix(m_pLightCamera->GetViewMatrixXM());
		m_ForwardEffect.SetShadowTextureArray(m_CSManager.GetCascadesOutput());

		m_ForwardEffect.SetRenderDefault(true);
		m_Powerplant.Draw(m_pd3dImmediateContext.Get(), m_ForwardEffect);
		m_Cube.Draw(m_pd3dImmediateContext.Get(), m_ForwardEffect);

		m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
		m_ForwardEffect.SetShadowTextureArray(nullptr);
		m_ForwardEffect.Apply(m_pd3dImmediateContext.Get());

	}
	m_GpuTimer_Lighting.Stop();
}

void GameApp::RenderSkybox()
{
	m_GpuTimer_Skybox.Start();
	{
		D3D11_VIEWPORT skyboxViewport = m_pViewerCamera->GetViewPort();
		skyboxViewport.MinDepth = 1.0f;
		skyboxViewport.MaxDepth = 1.0f;
		m_pd3dImmediateContext->RSSetViewports(1, &skyboxViewport);

		m_SkyboxEffect.SetRenderDefault();
		m_SkyboxEffect.SetLitTexture(m_pLitBuffer->GetShaderResource());
		m_SkyboxEffect.SetDepthTexture(m_pDepthBuffer->GetShaderResource());

		// 由于全屏绘制，不需要用到深度缓冲区，也就不需要清空后备缓冲区了
		ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
		m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
		m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);

		m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
		m_SkyboxEffect.SetLitTexture(nullptr);
		m_SkyboxEffect.SetDepthTexture(nullptr);
		m_SkyboxEffect.Apply(m_pd3dImmediateContext.Get());
	}
	m_GpuTimer_Skybox.Stop();
}