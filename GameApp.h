#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Camera.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "DDSTextureLoader.h"

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

		//获取位置
		DirectX::XMFLOAT3 GetPosition() const;
		// 获取物体变换
		Transform& GetTransform();
		// 获取物体变换
		const Transform& GetTransform() const;

		//设置缓冲区
		template <class VertexType, class IndexType>
		void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);
		//设置纹理
		void SetTexture(ID3D11ShaderResourceView* texture);
		//绘制
		void Draw(ID3D11DeviceContext* deviceContext);

		// 设置调试对象名
		// 若缓冲区被重现设置，调试对象名也需要被重新设置
		void SetDebugObjectName(const std::string& name);
	private:
		Transform m_Transform;							//世界矩阵
		ComPtr<ID3D11ShaderResourceView> m_pTexture;	//纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;			//顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;			//索引缓冲区
		UINT m_VertexStride;							//顶点字节大小
		UINT m_IndexCount;								//索引数目
	};

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
	bool InitEffect();
	bool InitResource();

private:

	ComPtr<ID3D11InputLayout> m_pVertexLayout2D;	// 2D顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;	// 3D顶点输入布局
	ComPtr<ID3D11Buffer> m_pConstantBuffers[4];	    // 常量缓冲区

	GameObject m_WoodCrate;							// 木盒
	GameObject m_Floor;								// 地板
	std::vector<GameObject> m_Walls;				// 墙壁

	ComPtr<ID3D11VertexShader> m_pVertexShader2D;	// 2D顶点着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader3D;	// 3D顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;		// 2D像素着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;		// 3D像素着色器

	CBChangesEveryFrame m_CBFrame;					// 该缓冲区存放仅在每一帧进行更新的变量
	CBChangesOnResize m_CBOnResize;					// 该缓冲区存放仅在窗口大小变化时更新的变量
	CBChangesRarely m_CBRarely;						// 该缓冲区存放不再进行修改的变量

	ComPtr<ID3D11SamplerState> m_pSamplerState;		//采样器状态

	std::shared_ptr<Camera> m_pCamera;				//摄像机
	CameraMode m_CameraMode;						//摄像机魔兽
};

#endif

template<class VertexType, class IndexType>
inline void GameApp::GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	//设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	//新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	//设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	InitData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
}
