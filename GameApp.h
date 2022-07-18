#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <Camera.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
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

	//std::unique_ptr<Depth2D>

	ComPtr<ID3D11Buffer> mPointSpritesBuffer;					// 点精灵顶点缓冲区
	ComPtr<ID3D11ShaderResourceView> mTreeTexArray;				// 树的纹理数组
	Material m_TreeMat;											// 树的材质

	BasicEffect m_BasicEffect;									// 对象渲染特效管理

	GameObject m_Floor;										    // 地板

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式

	bool m_FogEnabled;										    // 是否开启雾效
	bool m_IsNight;											    // 是否黑夜
	bool m_EnableAlphaToCoverage;							    // 是否开启Alpha-To-Coverage

	float m_FogRange;										    // 雾效范围
};

#endif