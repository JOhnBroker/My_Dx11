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

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	// 球体的模式
	enum class SphereMode { None, Reflection, Refraction };

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();
	void DrawScene(bool drawCenterSphere, bool enableGS, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, ID3D11ShaderResourceView* pSRV);

private:
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;

	BasicEffect m_BasicEffect;											// 对象渲染特效管理
	SkyBoxEffect m_SkyboxEffect;										// 天空盒特效管理

	std::unique_ptr<Depth2D> m_pDepthTexture;							// 深度缓冲区
	std::unique_ptr<TextureCube> m_pDynamicTextureCube;					// 动态天空盒
	std::unique_ptr<Depth2D> m_pDynamicCubeDepthTexture;				// 渲染动态天空盒的深度缓冲区
	std::unique_ptr<Texture2D> m_pDebugDynamicCubeTexture;			// 调试动态天空盒用

	std::unique_ptr<TextureCube> m_pDynamicSkyboxGS;
	std::unique_ptr<Depth2DArray> m_pDepthArray;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSRV = nullptr;

	GameObject m_Sphere[5];												// 球
	GameObject m_CenterSphere;											// 中心球
	GameObject m_Cylinder[5];											// 圆柱
	GameObject m_Ground;												// 地面
	GameObject m_Skybox;												// 天空盒
	GameObject m_DebugSkybox;											// 调试用天空盒

	std::shared_ptr<FirstPersonCamera> m_pCamera;						// 摄像机
	std::shared_ptr<FirstPersonCamera> m_pCubeCamera;					// 动态天空盒的摄像机
	std::shared_ptr<FirstPersonCamera> m_pDebugCamera;					// 调试动态天空盒的摄像机
	FirstPersonCameraController m_CameraController;						// 摄像机控制器

	ImVec2 m_DebugTextureXY;											// 调试显示纹理的位置
	ImVec2 m_DebugTextureWH;											// 调试显示纹理的宽高

	SphereMode m_SphereMode = SphereMode::Reflection;					// 中心球渲染模式
	float m_SphereRad = 0.0f;											// 球体旋转弧度
	float m_Eta = 1.0f / 1.51f;											// 空气/介质折射率
	bool m_UseGS = false;

};

#endif