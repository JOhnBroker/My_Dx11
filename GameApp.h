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

#include <ScreenGrab11.h>
#include <wincodec.h>

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
	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

private:
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;

	BasicEffect m_BasicEffect;											// 对象渲染特效管理
	SkyBoxEffect m_SkyBoxEffect;									// 天空盒理特效管理

	std::unique_ptr<Depth2D> m_pDepthTexture;							// 深度缓冲区
	std::unique_ptr<TextureCube> m_pDynamicTextureCube;
	std::unique_ptr<Depth2D> m_pDynamicCubeDepthTexture;

	GameObject m_Spheres[5];
	GameObject m_Ground;												// 地面
	GameObject m_CenterSphere;
	GameObject m_Cylinders[5];
	GameObject m_Skybox;

	std::shared_ptr<FirstPersonCamera> m_pCamera;						// 摄像机
	std::shared_ptr<FirstPersonCamera> m_pCubeCamera;						// 摄像机
	FirstPersonCameraController m_CameraController;						// 摄像机控制器

	float m_SphereRad = 0.0f;									// 球体旋转弧度

	bool m_EnableNormalMap = true;								// 开启法线贴图

	GroundMode m_GroundMode = GroundMode::Floor;                // 哪种地面类型
};

#endif