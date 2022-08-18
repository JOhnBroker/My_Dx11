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
#include <SSAOManager.h>
#include <Waves.h>

#include <ScreenGrab11.h>
#include <wincodec.h>

#include <random>
#include <algorithm>

#include <sstream>

#define BITONIC_BLOCK_SIZE 512

#define TRANSPOSE_BLOCK_SIZE 16

class GameApp : public D3DApp
{
public:
	enum class RenderMode { Basic, NormalMap, DisplacementMap };

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void RenderSSAO();
	void RenderShadow();
	void RenderForward();
	void RenderSkybox();

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
	bool InitResource();
	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

private:
	RenderMode m_RenderMode = RenderMode::DisplacementMap;
	RasterizerMode m_RasterizerMode = RasterizerMode::Solid;
	int m_HeightScale = 7;

	TextureManager m_TextureManager;
	ModelManager m_ModelManager;
	SSAOManager m_SSAOManager;

	bool m_EnableSSAO = true;
	bool m_UpdateLight = true;
	bool m_EnableDebug = true;

	GameObject m_Ground;
	GameObject m_Cylinders[10];
	GameObject m_Spheres[10];
	GameObject m_House;
	GameObject m_Skybox;

	std::unique_ptr<Depth2D> m_pDepthTexture;
	std::unique_ptr<Texture2D> m_pLitTexture;
	std::unique_ptr<Depth2D> m_pShadowMapTexture;
	std::unique_ptr<Texture2D> m_pDebugAOTexture;

	DirectionalLight m_DirLights[3] = {};
	DirectX::XMFLOAT3 m_OriginalLightDirs[3] = {};

	BasicEffect m_BasicEffect;
	ShadowEffect m_ShadowEffect;
	SkyBoxEffect m_SkyboxEffect;
	SSAOEffect m_SSAOEffect;

	std::shared_ptr<FirstPersonCamera> m_pCamera;
	FirstPersonCameraController m_CameraController;
};

#endif