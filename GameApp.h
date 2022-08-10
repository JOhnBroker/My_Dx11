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
#include <Waves.h>

#include <ScreenGrab11.h>
#include <wincodec.h>

#include <random>
#include <algorithm>

#define BITONIC_BLOCK_SIZE 512

#define TRANSPOSE_BLOCK_SIZE 16

struct FragmentData
{
	uint32_t color;
	float depth;
};

struct FLStaticNode
{
	FragmentData data;
	uint32_t next;
};

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	// 球体的模式
	enum class SphereMode { None, Reflection, Refraction };
	// 地面的模式
	enum class GroundMode { Floor, Stones };

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

	void RenderShadow();
	void RenderForward();
	void RenderSkybox();

	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);
	template<class Effect>
	void DrawScene(Effect& effect, std::function<void(Effect&, ID3D11DeviceContext*)>fun = [](Effect&, ID3D11DeviceContext*) {})
	{
		// 有法线贴图
		{
			m_Ground.Draw(m_pd3dImmediateContext.Get(), effect);

			for (auto& cylinder : m_Cylinders)
			{
				cylinder.Draw(m_pd3dImmediateContext.Get(), effect);
			}
		}

		// 没有法线贴图
		fun(effect, m_pd3dImmediateContext.Get());
		// 石球
		for (auto& sphere : m_Spheres)
		{
			sphere.Draw(m_pd3dImmediateContext.Get(), effect);
		}
		// 房屋
		m_House.Draw(m_pd3dImmediateContext.Get(), effect);
	};

private:
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;

	bool m_UpdateLight = true;
	bool m_EnableNormalMap = true;
	bool m_EnableDebug = true;
	int m_SlopeIndex = 0;

	BasicEffect m_BasicEffect;
	SkyBoxEffect m_SkyboxEffect;
	ShadowEffect m_ShadowEffect;

	GameObject m_Ground;
	GameObject m_Cylinders[10];
	GameObject m_Spheres[10];
	GameObject m_House;
	GameObject m_Skybox;

	std::unique_ptr<Depth2D> m_pDepthTexture;
	std::unique_ptr<Texture2D> m_pLitTexture;
	std::unique_ptr<Depth2D> m_pShadowMapTexture;
	std::unique_ptr<Texture2D> m_pDebugShadowTexture;

	DirectionalLight m_DirLights[3] = {};
	DirectX::XMFLOAT3 m_OriginalLightDirs[3] = {};

	std::shared_ptr<FirstPersonCamera> m_pCamera;
	FirstPersonCameraController m_CameraController;
};

#endif