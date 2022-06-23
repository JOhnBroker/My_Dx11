#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
	Camera() = default;
	virtual ~Camera() = 0;

	//获取摄像机位置
	DirectX::XMVECTOR GetPositionXM() const;
	DirectX::XMVECTOR GetPosition() const;

	//获取摄像机旋转
	float GetRotationX() const;
	float GetRotationY() const;

	//获取摄像机的坐标轴向量
	DirectX::XMVECTOR GetRightAxisXM() const;
	DirectX::XMFLOAT3 GetRightAxis()const;
	DirectX::XMVECTOR GetUpAxisXM() const;
	DirectX::XMFLOAT3 GetUpAxis()const;
	DirectX::XMVECTOR GetLookAxisXM() const;
	DirectX::XMFLOAT3 GetLookAxis()const;

	//获取矩阵
	DirectX::XMMATRIX GetViewXM() const;
	DirectX::XMMATRIX GetProjXM() const;
	DirectX::XMMATRIX GetViewProjXM() const;

	//获取视口
	D3D11_VIEWPORT GetViewPort() const;

	//设置视锥体
	void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

	//设置视口
	void SetViewPort(const D3D11_VIEWPORT& viewPort);
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

protected:
	//摄像机的变换
	Transform m_Transfrom = {};
	//视锥体属性
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;

	//当前视口
	D3D11_VIEWPORT m_ViewPort = {};
};



#endif
