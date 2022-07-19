#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <Camera.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
//#include <Buffer.h>
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

	std::unique_ptr<Depth2D> m_pDepthTexture;					// 深度缓冲贴图

	GameObject m_House;											// 房子
	GameObject m_Floor;										    // 地板

	std::shared_ptr<ThirdPersonCamera> m_pCamera;				// 摄像机

};

#endif