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
	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

private:
	TextureManager m_TextureManager;
	ModelManager m_ModelManager;

	std::mt19937 m_RandEngine;
	std::uniform_int_distribution<uint32_t> m_RowRange;
	std::uniform_int_distribution<uint32_t> m_ColRange;
	std::uniform_real_distribution<float> m_MagnitudeRange;

	BasicEffect m_BasicEffect;
	PostProcessEffect m_PostProcessEffect;

	GameObject m_Land;
	GameObject m_RedBox;
	GameObject m_YellowBox;
	GpuWaves m_GpuWaves;

	std::unique_ptr<StructuredBuffer<FLStaticNode>> m_pFLStaticNodeBuffer;
	std::unique_ptr<ByteAddressBuffer> m_pStartOffsetBuffer;

	std::unique_ptr<Depth2D> m_pDepthTexture;
	std::unique_ptr<Texture2D> m_pLitTexture;
	std::unique_ptr<Texture2D> m_pTempTexture;

	float m_BaseTime = 0.0f;
	bool m_EnabledFog = true;
	bool m_EnabledOIT = true;
	int m_BlurMode = 1;

	float m_BlurSigma = 2.5f;
	int m_BlurRadius = 5;
	int m_BlurTimes = 1;

	std::shared_ptr<Camera> m_pCamera;

};

#endif