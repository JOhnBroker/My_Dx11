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

#include <random>
#include <algorithm>

#define BITONIC_BLOCK_SIZE 512

#define TRANSPOSE_BLOCK_SIZE 16

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	// 球体的模式
	enum class SphereMode { None, Reflection, Refraction };
	// 地面的模式
	enum class GroundMode { Floor, Stones };

	struct CB
	{
		UINT level;
		UINT descendMask;
		UINT matrixWidth;
		UINT matrixHeight;
	};

public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	void Compute();

private:
	bool InitResource();
	void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);
	void SetConstants(UINT level, UINT descendMask, UINT matrixWidth, UINT matrixHeight);
	void GPUSort();

private:
	ComPtr<ID3D11Buffer> m_pConstantBuffer;
	ComPtr<ID3D11Buffer> m_pTypedBuffer1;
	ComPtr<ID3D11Buffer> m_pTypedBuffer2;
	ComPtr<ID3D11Buffer> m_pTypedBufferCopy;
	ComPtr<ID3D11UnorderedAccessView> m_pDataUAV1;
	ComPtr<ID3D11UnorderedAccessView> m_pDataUAV2;
	ComPtr<ID3D11ShaderResourceView> m_pDataSRV1;
	ComPtr<ID3D11ShaderResourceView> m_pDataSRV2;

	std::vector<UINT> m_RandomNums;
	UINT m_RandomNumsCount = 0;
	ComPtr<ID3D11ComputeShader>m_pBitonicSort_CS;
	ComPtr<ID3D11ComputeShader>m_pMatrixTranspose_CS;

	GpuTimer m_GpuTimer;

};

#endif