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
	enum class TessellationMode { Triangle, Quad, BezierCurve, BezierSurface };
	enum class PartitionMode { Integer, Odd, Even };

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void UpdateTriangle();
	void UpdateQuad();
	void UpdateBezierCurve();
	void UpdateBezierSurface();

	void DrawTriangle();
	void DrawQuad();
	void DrawBezierCurve();
	void DrawBezierSurface();

	void RenderSSAO();
	void RenderShadow();
	void RenderForward();
	void RenderSkybox();

	//template<class Effect>
	//void DrawScene(Effect& effect, std::function<void(Effect&, ID3D11DeviceContext*)>fun = [](Effect&, ID3D11DeviceContext*) {})
	//{
	//	// 有法线贴图
	//	{
	//		m_Ground.Draw(m_pd3dImmediateContext.Get(), effect);
	//
	//		for (auto& cylinder : m_Cylinders)
	//		{
	//			cylinder.Draw(m_pd3dImmediateContext.Get(), effect);
	//		}
	//	}
	//
	//	// 没有法线贴图
	//	fun(effect, m_pd3dImmediateContext.Get());
	//	// 石球
	//	for (auto& sphere : m_Spheres)
	//	{
	//		sphere.Draw(m_pd3dImmediateContext.Get(), effect);
	//	}
	//	// 房屋
	//	m_House.Draw(m_pd3dImmediateContext.Get(), effect);
	//};

private:
	bool InitResource();
	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

private:
	TessellationMode m_TessellationMode = TessellationMode::Triangle;
	PartitionMode m_PartitionMode = PartitionMode::Integer;

	int m_ChosenBezPoint = -1;
	ComPtr<ID3D11Buffer> m_pBezCurveVB;
	DirectX::XMFLOAT3 m_BezPoints[10] = {};
	float m_IsolineEdgeTess[2] = { 1.0f,64.0f };

	ComPtr<ID3D11Buffer> m_pTriangleVB;
	float m_TriEdgeTess[3] = { 4.0f,4.0f,4.0f };
	float m_TriInsideTess = 4.0f;

	ComPtr<ID3D11Buffer> m_pQuadVB;
	float m_QuadEdgeTess[4] = { 4.0f,4.0f,4.0f,4.0f };
	float m_QuadInsideTess[2] = { 4.0f,4.0f };

	ComPtr<ID3D11Buffer> m_pBezSurfaceVB;
	float m_QuadPatchEdgeTess[4] = { 25.0f,25.0f,25.0f,25.0f };
	float m_QuadPatchInsideTess[2] = { 25.0f,25.0f };
	float m_Phi = 3.14159f / 4, m_Theta = 0.0f, m_Radius = 30.0f;

	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11RasterizerState> m_pRSWireFrame;

	std::unique_ptr<EffectHelper> m_pEffectHelper;
};

#endif