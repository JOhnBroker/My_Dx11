#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"

class GameApp : public D3DApp
{
public:
	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;

	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT4 eyePos;
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely
	{
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		Material material;
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;
	};

	class GameObject
	{
	public:
		GameObject();
	};

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
	bool InitEffect();
	bool InitResource();

	template<class VertexType>
	bool ResetMesh(const Geometry::MeshData<VertexType>& meshData);


private:

	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;	    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;					// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;		// 文本格式

	ComPtr<ID3D11InputLayout> m_pVertexLayout2D;	// 2D顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;	// 3D顶点输入布局
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
	ComPtr<ID3D11Buffer> m_pConstantBuffers[2];	    // 常量缓冲区
	UINT m_IndexCount;							    // 绘制物体的索引数组大小
	int m_CurrFrame;
	ShowMode m_CurrMode;

	ComPtr<ID3D11ShaderResourceView> m_pWoodCrate;
	ComPtr<ID3D11ShaderResourceView> m_pFire;
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_pFireAnimes;
	ComPtr<ID3D11SamplerState> m_pSamplerState;

	ComPtr<ID3D11VertexShader> m_pVertexShader2D;	// 2D顶点着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader3D;	// 3D顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;		// 2D像素着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;		// 3D像素着色器
	VSConstantBuffer m_VSConstantBuffer;			// 用于修改用于VS的GPU常量缓冲区的变量
	PSConstantBuffer m_PSConstantBuffer;			// 用于修改用于PS的GPU常量缓冲区的变量
};


#endif