#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <ctime>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>
#include "CascadedShadowManager.h"

// 需要与着色器中的PointLight对应

struct PointLight
{
	DirectX::XMFLOAT3 posV;
	float attenuationBegin;
	DirectX::XMFLOAT3 color;
	float attenuationEnd;
};

// 初始变换数据(柱面坐标系)
struct PointLightInitData
{
	float radius;
	float angle;
	float height;
	float animationSpeed;
};

class GameApp : public D3DApp
{
public:
	enum class RenderMode { Basic, NormalMap, DisplacementMap };
	enum class LightCullTechnique {
		CULL_FORWARD_NONE = 0,					// 前向渲染，无光照裁剪
		CULL_FORWARD_PREZ_NONE,					// 前向渲染，预写入深度，无光照裁剪
		CULL_FORWARD_COMPUTE_SHADER_TILE,		// 前向渲染，预写入深度，Tile-Based 光照裁剪
		CULL_FORWARD_COMPUTE_SHADER_2_5D,		// 前向渲染，预写入深度，Tile-Based 2.5D光照裁剪
		CULL_DEFERRED_NONE,						// 传统延迟渲染
		CULL_DEFERRED_COMPUTE_SHADER_TILE		// Tile-Based 延迟渲染
	};

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth = 1280, int initHeight = 720);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	template<class Effect>
	void DrawScene(Effect& effect, std::function<void(Effect&, ID3D11DeviceContext*)>fun = [](Effect&, ID3D11DeviceContext*) {})
	{
		//// 有法线贴图
		//{
		//	m_Ground.Draw(m_pd3dImmediateContext.Get(), effect);
		//
		//	for (auto& cylinder : m_Cylinders)
		//	{
		//		cylinder.Draw(m_pd3dImmediateContext.Get(), effect);
		//	}
		//}
		//
		//// 没有法线贴图
		//fun(effect, m_pd3dImmediateContext.Get());
		//// 石球
		//for (auto& sphere : m_Spheres)
		//{
		//	sphere.Draw(m_pd3dImmediateContext.Get(), effect);
		//}
		//// 房屋
		//m_House.Draw(m_pd3dImmediateContext.Get(), effect);
	};

private:
	bool InitResource();
	//void InitLightParams();

	//DirectX::XMFLOAT3 HueToRGB(float hue);

	//void ResizeLights(UINT activeLights);
	//void UpdateLights(float dt);
	//
	//void XM_CALLCONV SetupLights(DirectX::XMMATRIX viewMatrix);

	//void ResizeBuffers(UINT width, UINT height, UINT msaaSamples);

	//void RenderForward(bool doPreZ);
	//void RenderGBuffer();
	void RenderShadowForAllCascades();
	void RenderForward();
	void RenderSkybox();
	//void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

private:

	// GPU计时
	GpuTimer m_GpuTimer_Shadow;
	GpuTimer m_GpuTimer_Lighting;
	GpuTimer m_GpuTimer_Skybox;

	// 设置
	int m_MsaaSamples = 1;
	int m_DebugShadowIndex = 1;

	// 阴影
	CascadedShadowManager m_CSManager;
	bool m_DebugShadow = false;

	// 资源
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;
	std::unique_ptr<Texture2DMS> m_pLitBuffer;
	std::unique_ptr<Depth2DMS> m_pDepthBuffer;
	std::unique_ptr<Texture2D> m_pDebugShadowBuffer;

	// 模型
	GameObject m_Powerplant;
	GameObject m_Cube;
	GameObject m_Skybox;

	// 特效
	ForwardEffect m_ForwardEffect;
	ShadowEffect m_ShadowEffect;
	SkyBoxEffect m_SkyboxEffect;

	// 摄像机
	std::shared_ptr<Camera> m_pViewerCamera;
	std::shared_ptr<Camera> m_pLightCamera;
	FirstPersonCameraController m_FPSCameraController;
};

#endif