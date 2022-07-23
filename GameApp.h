#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <ctime>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <Camera.h>
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

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

private:
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;

	BasicEffect m_BasicEffect;

	std::unique_ptr<Depth2D> m_pDepthTexture;						// 深度缓冲贴图

	int m_SceneMode = 0;

	GameObject m_House;												// 树
	GameObject m_Cubes;												// 立方体
	GameObject m_Sphere;
	GameObject m_Cylinder;
	GameObject m_Triangle;
	DirectX::BoundingSphere m_BoundingSphere;
	
	GeometryData m_TriangleMesh;

	std::shared_ptr<FirstPersonCamera> m_pCamera;				// 摄像机

};

#endif