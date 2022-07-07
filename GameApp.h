#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <d3dApp.h>
#include <Camera.h>
#include "GameObject.h"

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	//enum class CameraMode { FirstPerson, ThirdPerson, Free };
	enum class Mode { SplitedTriangle, SplitedSnow, SplitedSphere };

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

	void ResetTriangle();
	void ResetRoundWire();

	void ResetSplitedTriangle();
	void ResetSplitedSnow();
	void ResetSplitedSphere();

private:
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;		// 单色笔刷
	ComPtr<IDWriteFont> m_pFont;					// 字体
	ComPtr<IDWriteTextFormat> m_pTextFront;			// 文本格式

	ComPtr<ID3D11Buffer> m_pVertexBuffers[7];		// 顶点缓冲区数组
	//int m_VertexCount;								// 顶点数目
	int m_InitVertexCounts;							// 初始顶点数目

	Mode m_ShowMode;								// 当前显示模式
	bool m_IsWireFrame;								// 是否为线框模式
	bool m_ShowNormal;								// 是否显示法向量
	int m_CurrIndex;								// 当前索引

	BasicEffect m_BasicEffect;						// 对象渲染特效管理

	//GameObject m_WoodCrate;									    // 木盒
	//GameObject m_Floor;										    // 地板
	//std::vector<GameObject> m_Walls;							// 墙壁
	//GameObject m_Mirror;										// 镜面
	//GameObject m_BoltAnim;										// 闪电动画
	//
	//std::vector<ComPtr<ID3D11ShaderResourceView>> m_BoltSRVs;	//闪电动画纹理
	//
	//Material m_ShadowMat;									    // 阴影材质
	//Material m_WoodCrateMat;									// 木盒材质
	//
	//BasicEffect m_BasicEffect;								    // 对象渲染特效管理
	//
	//std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	//CameraMode m_CameraMode;									// 摄像机模式

};

#endif