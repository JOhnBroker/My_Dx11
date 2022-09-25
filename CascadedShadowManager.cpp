#include "CascadedShadowManager.h"

using namespace DirectX;

HRESULT CascadedShadowManager::InitResource(ID3D11Device* device)
{
	DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT;
	if (m_ShadowBits == 4) 
	{
		switch (m_ShadowType)
		{
		case ShadowType::ShadowType_ESM:
		case ShadowType::ShadowType_CSM:format = DXGI_FORMAT_R32_FLOAT; break;
		case ShadowType::ShadowType_VSM:
		case ShadowType::ShadowType_EVSM2:format = DXGI_FORMAT_R32G32_FLOAT; break;
		case ShadowType::ShadowType_EVSM4:format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		}
	}
	else if (m_ShadowBits == 2) 
	{
		switch (m_ShadowType)
		{
		case ShadowType::ShadowType_ESM:format = DXGI_FORMAT_R16_FLOAT; break;
		case ShadowType::ShadowType_CSM:format = DXGI_FORMAT_R16_UNORM; break;
		case ShadowType::ShadowType_VSM:format = DXGI_FORMAT_R16G16_UNORM; break;
		case ShadowType::ShadowType_EVSM2:format = DXGI_FORMAT_R16G16_FLOAT; break;
		case ShadowType::ShadowType_EVSM4:format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
		}
	}

	m_pCSMTextureArray = std::make_unique<Texture2DArray>(device, m_ShadowSize, m_ShadowSize, format
		, (uint32_t)m_CascadeLevels, m_GenerateMips ? (int)log2f((float)m_ShadowSize) + 1 : 1);
	m_pCSMTempTexture = std::make_unique<Texture2D>(device, m_ShadowSize, m_ShadowSize, format, m_GenerateMips ? (int)log2f((float)m_ShadowSize) + 1 : 1);
	
	m_pCSMDepthBuffer = std::make_unique<Depth2D>(device, m_ShadowSize, m_ShadowSize, DepthStencilBitsFlag::Depth_32Bits);
	
	m_ShadowViewport.TopLeftX = 0;
	m_ShadowViewport.TopLeftY = 0;
	m_ShadowViewport.Width = (float)m_ShadowSize;
	m_ShadowViewport.Height = (float)m_ShadowSize;
	m_ShadowViewport.MinDepth = 0.0f;
	m_ShadowViewport.MaxDepth = 1.0f;

	m_pCSMTempTexture->SetDebugObjectName("CSM Temp Texture");
	m_pCSMTextureArray->SetDebugObjectName("CSM Texture Array");
	m_pCSMDepthBuffer->SetDebugObjectName("CSM Depth Buffer");

	return S_OK;
}

void CascadedShadowManager::UpdateFrame(const Camera& viewerCamera, const Camera& lightCamera, const DirectX::BoundingBox& sceneBoundingBox)
{
	XMMATRIX ViewerProj = viewerCamera.GetProjMatrixXM();
	XMMATRIX ViewerView = viewerCamera.GetViewMatrixXM();
	XMMATRIX LightView = lightCamera.GetViewMatrixXM();
	XMMATRIX ViewerInvView = XMMatrixInverse(nullptr, ViewerView);

	float frustumIntervalBegin, frustumIntervalEnd;
	XMVECTOR lightCameraOrthographicMinVec;
	XMVECTOR lightCameraOrthographicMaxVec;
	float cameraNearFarRange = viewerCamera.GetFarZ() - viewerCamera.GetNearZ();

	XMVECTOR worldUnitsPerTexelVec = g_XMZero;
	//
	// 为每个级联计算光照空间下的正交投影矩阵
	//
	for (int cascadeIndex = 0; cascadeIndex < m_CascadeLevels; ++cascadeIndex)
	{
		if (m_SelectedCascadesFit == FitProjection::FitProjection_ToCascade)
		{
			if (cascadeIndex == 0)
				frustumIntervalBegin = 0.0f;
			else
				frustumIntervalBegin = m_CascadePartitionsPercentage[cascadeIndex - 1];
		}
		else
		{
			// Map-Based
			frustumIntervalBegin = 0.0f;
		}
		// 算出视锥体Z区间
		frustumIntervalEnd = m_CascadePartitionsPercentage[cascadeIndex];
		frustumIntervalBegin = frustumIntervalBegin * cameraNearFarRange;
		frustumIntervalEnd = frustumIntervalEnd * cameraNearFarRange;

		XMFLOAT3 viewerFrustumPoints[8];
		BoundingFrustum viewerFrustum(ViewerProj);
		viewerFrustum.Near = frustumIntervalBegin;
		viewerFrustum.Far = frustumIntervalEnd;

		// 将局部视锥体变换到世界空间后，再变换到光照空间
		viewerFrustum.Transform(viewerFrustum, ViewerInvView * LightView);
		viewerFrustum.GetCorners(viewerFrustumPoints);
		// 计算视锥体在光照空间下的AABB和vMax, vMin
		BoundingBox viewerFrustumBox;
		BoundingBox::CreateFromPoints(viewerFrustumBox, 8, viewerFrustumPoints, sizeof(XMFLOAT3));
		lightCameraOrthographicMaxVec = XMLoadFloat3(&viewerFrustumBox.Center) + XMLoadFloat3(&viewerFrustumBox.Extents);
		lightCameraOrthographicMinVec = XMLoadFloat3(&viewerFrustumBox.Center) - XMLoadFloat3(&viewerFrustumBox.Extents);

		// 用于 消除由于光线改变或摄像机视角变化导致阴影边缘出现的闪烁效果
		if (m_FixedSizeFrustumAABB)
		{
			// 使用max(子视锥体的斜对角线, 远平面对角线)的长度作为XY的宽高，从而固定AABB的宽高
		   // 并且它的宽高足够大，且总是能覆盖当前级联的视锥体

		   //     Near    Far
		   //    0----1  4----5
		   //    |    |  |    |
		   //    |    |  |    |
		   //    3----2  7----6
			XMVECTOR diagVec = XMLoadFloat3(viewerFrustumPoints + 7) - XMLoadFloat3(viewerFrustumPoints + 1);		// 子视锥体的斜对角线
			XMVECTOR diag2Vec = XMLoadFloat3(viewerFrustumPoints + 7) - XMLoadFloat3(viewerFrustumPoints + 5);		// 远平面对角线
			XMVECTOR lengthVec = XMVectorMax(XMVector3Length(diagVec), XMVector3Length(diag2Vec));

			// 计算出的偏移量会填充正交投影
			XMVECTOR borderOffsetVec = (lengthVec - (lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec)) * g_XMOneHalf.v;
			static const XMVECTORF32 xyzw1100Vec = { {1.0f,1.0f,0.0f,0.0f} };
			lightCameraOrthographicMaxVec += borderOffsetVec * xyzw1100Vec.v;
			lightCameraOrthographicMinVec -= borderOffsetVec * xyzw1100Vec.v;
		}

		// 我们基于PCF核的大小再计算一个边界扩充值使得包围盒稍微放大一些。
		// 等比缩放不会影响前面固定大小的AABB
		{
            float scaleDuetoBlur = m_BlurKernelSize / (float)m_ShadowSize;
            XMVECTORF32 scaleDuetoBlurVec = { {scaleDuetoBlur, scaleDuetoBlur, 0.0f, 0.0f} };

			XMVECTOR borderOffsetVec = lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec;
            borderOffsetVec *= g_XMOneHalf.v;
            borderOffsetVec *= scaleDuetoBlurVec.v;
			lightCameraOrthographicMaxVec += borderOffsetVec;
			lightCameraOrthographicMinVec -= borderOffsetVec;
		}

		if (m_MoveLightTexelSize)
		{
			// 计算阴影图中每个texel对应世界空间的宽高，用于后续避免阴影边缘的闪烁
			float normalizeByBufferSize = 1.0f / m_ShadowSize;
			XMVECTORF32 normalizeByBufferSizeVec = { {normalizeByBufferSize,normalizeByBufferSize,0.0f,0.0f} };
			worldUnitsPerTexelVec = lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec;
			worldUnitsPerTexelVec *= normalizeByBufferSize;

			// 对齐Texel，使得移动摄像机的时候不会出现阴影的抖动
			lightCameraOrthographicMinVec /= worldUnitsPerTexelVec;
			lightCameraOrthographicMinVec = XMVectorFloor(lightCameraOrthographicMinVec);
			lightCameraOrthographicMinVec *= worldUnitsPerTexelVec;
			lightCameraOrthographicMaxVec /= worldUnitsPerTexelVec;
			lightCameraOrthographicMaxVec = XMVectorFloor(lightCameraOrthographicMaxVec);
			lightCameraOrthographicMaxVec *= worldUnitsPerTexelVec;
		}

		float nearPlane = 0.0f;
		float farPlane = 0.0f;

		// 将场景AABB的角点变换到光照空间
		XMVECTOR sceneAABBPointsLightSpace[8]{};
		{
			XMFLOAT3 corners[8];
			sceneBoundingBox.GetCorners(corners);
			for (int i = 0; i < 8; ++i)
			{
				XMVECTOR v = XMLoadFloat3(corners + i);
                sceneAABBPointsLightSpace[i] = XMVector3Transform(v, LightView);
			}
		}
		if (m_SelectedNearFarFit == FitNearFar::FitNearFar_ZeroOne)
		{
			nearPlane = 0.1f;
			farPlane = 10000.0f;
		}
		if (m_SelectedNearFarFit == FitNearFar::FitNearFar_CascadeAABB)
		{
			nearPlane = XMVectorGetZ(lightCameraOrthographicMinVec);
			farPlane = XMVectorGetZ(lightCameraOrthographicMaxVec);
		}
		else if (m_SelectedNearFarFit == FitNearFar::FitNearFar_SceneAABB)
		{
			XMVECTOR lightSpaceSceneAABBminValueVec = g_XMFltMax.v;
			XMVECTOR lightSpaceSceneAABBmaxValueVec = -g_XMFltMax.v;

			for (int i = 0; i < 8; ++i)
			{
				lightSpaceSceneAABBminValueVec = XMVectorMin(sceneAABBPointsLightSpace[i], lightSpaceSceneAABBminValueVec);
				lightSpaceSceneAABBmaxValueVec = XMVectorMax(sceneAABBPointsLightSpace[i], lightSpaceSceneAABBmaxValueVec);
			}
			nearPlane = XMVectorGetZ(lightSpaceSceneAABBminValueVec);
			farPlane = XMVectorGetZ(lightSpaceSceneAABBmaxValueVec);
		}
		else if (m_SelectedNearFarFit == FitNearFar::FitNearFar_SceneAABB_Intersection)
		{
			ComputeNearAndFar(nearPlane, farPlane, lightCameraOrthographicMinVec, lightCameraOrthographicMaxVec, sceneAABBPointsLightSpace);
		}

		XMStoreFloat4x4(m_ShadowProj + cascadeIndex, XMMatrixOrthographicOffCenterLH(
			XMVectorGetX(lightCameraOrthographicMinVec), XMVectorGetX(lightCameraOrthographicMaxVec),
			XMVectorGetY(lightCameraOrthographicMinVec), XMVectorGetY(lightCameraOrthographicMaxVec),
			nearPlane, farPlane));

		// 创建最终的正交投影AABB
		lightCameraOrthographicMinVec = XMVectorSetZ(lightCameraOrthographicMinVec, nearPlane);
		lightCameraOrthographicMaxVec = XMVectorSetZ(lightCameraOrthographicMaxVec, farPlane);
		BoundingBox::CreateFromPoints(m_ShadowProjBoundingBox[cascadeIndex],
			lightCameraOrthographicMinVec, lightCameraOrthographicMaxVec);
		m_CascadePartitionsFrustum[cascadeIndex] = frustumIntervalEnd;
	}

}

struct Triangle
{
	XMVECTOR point[3];
	bool isCulled;
};

//--------------------------------------------------------------------------------------
// 计算一个准确的近平面/远平面可以减少surface acne和Peter-panning
// 通常偏移量会用于PCF滤波来解决阴影问题
// 而准确的近平面/远平面可以提升精度
// 这个概念并不复杂，但相交测试的代码比较复杂
//--------------------------------------------------------------------------------------
void XM_CALLCONV CascadedShadowManager::ComputeNearAndFar(float& outNearPlane, float& outFarPlane, DirectX::FXMVECTOR lightCameraOrthographicMinVec, DirectX::FXMVECTOR lightCameraOrthographicMaxVec, DirectX::XMVECTOR pointsInCameraView[])
{
	// 核心思想
	// 1. 对AABB的所有12个三角形进行迭代
	// 2. 每个三角形分别对正交投影的4个侧面进行裁剪。裁剪过程中可能会出现这些情况：
	//    - 0个点在该侧面的内部，该三角形可以剔除
	//    - 1个点在该侧面的内部，计算该点与另外两个点在侧面上的交点得到新三角形
	//    - 2个点在该侧面的内部，计算这两个点与另一个点在侧面上的交点，分裂得到2个新三角形
	//    - 3个点都在该侧面的内部
	//    遍历中的三角形与新生产的三角形都要进行剩余侧面的裁剪
	// 3. 在这些三角形中找到最小/最大的Z值作为近平面/远平面

	outNearPlane = FLT_MAX;
	outFarPlane = -FLT_MAX;
	Triangle triangleList[16]{};
	int numTriangles;
	//      4----5
	//     /|   /| 
	//    0-+--1 | 
	//    | 7--|-6
	//    |/   |/  
	//    3----2
	static const int all_indices[][3] = {
	   {4,7,6}, {6,5,4},
	   {5,6,2}, {2,1,5},
	   {1,2,3}, {3,0,1},
	   {0,3,7}, {7,4,0},
	   {7,3,2}, {2,6,7},
	   {0,4,5}, {5,1,0}
	};
	bool triPointPassCollision[3]{};
	const float minX = XMVectorGetX(lightCameraOrthographicMinVec);
	const float maxX = XMVectorGetX(lightCameraOrthographicMaxVec);
	const float minY = XMVectorGetY(lightCameraOrthographicMinVec);
	const float maxY = XMVectorGetY(lightCameraOrthographicMaxVec);

	for (auto& indices : all_indices)
	{
		triangleList[0].point[0] = pointsInCameraView[indices[0]];
		triangleList[0].point[1] = pointsInCameraView[indices[1]];
		triangleList[0].point[2] = pointsInCameraView[indices[2]];
		numTriangles = 1;
		triangleList[0].isCulled = false;

		// 每个三角形都需要对4个视锥体侧面进行裁剪
		for (int planeIndex = 0; planeIndex < 4; ++planeIndex)
		{
			float edge;
			int component;
			switch (planeIndex)
			{
			case 0: edge = minX; component = 0; break;
			case 1: edge = maxX; component = 0; break;
			case 2: edge = minY; component = 1; break;
			case 3: edge = maxY; component = 1; break;
			default: break;
			}
			for (int triIdx = 0; triIdx < numTriangles; ++triIdx)
			{
				// 跳过裁剪的三角形
				if (triangleList[triIdx].isCulled)
					continue;
				int insideVertexCount = 0;
				for (int triVtxIdx = 0; triVtxIdx < 3; ++triVtxIdx)
				{
					switch (planeIndex)
					{
					case 0: triPointPassCollision[triVtxIdx] = (XMVectorGetX(triangleList[triIdx].point[triVtxIdx]) > minX); break;
					case 1: triPointPassCollision[triVtxIdx] = (XMVectorGetX(triangleList[triIdx].point[triVtxIdx]) < maxX); break;
                    case 2: triPointPassCollision[triVtxIdx] = (XMVectorGetY(triangleList[triIdx].point[triVtxIdx]) > minY); break;
                    case 3: triPointPassCollision[triVtxIdx] = (XMVectorGetY(triangleList[triIdx].point[triVtxIdx]) < maxY); break;
					default: break;
					}
					insideVertexCount += triPointPassCollision[triVtxIdx];
				}

				// 将通过视锥体测试的点挪到数组前面
				if (triPointPassCollision[1] && !triPointPassCollision[0])
				{
					std::swap(triangleList[triIdx].point[0], triangleList[triIdx].point[1]);
					triPointPassCollision[0] = true;
					triPointPassCollision[1] = false;
				}
				if (triPointPassCollision[2] && !triPointPassCollision[1])
				{
					std::swap(triangleList[triIdx].point[1], triangleList[triIdx].point[2]);
					triPointPassCollision[1] = true;
					triPointPassCollision[2] = false;
				}
				if (triPointPassCollision[1] && !triPointPassCollision[0])
				{
					std::swap(triangleList[triIdx].point[0], triangleList[triIdx].point[1]);
					triPointPassCollision[0] = true;
					triPointPassCollision[1] = false;
				}

				triangleList[triIdx].isCulled = (insideVertexCount == 0);
				if (insideVertexCount == 1)
				{
					// 找出三角形与当前平面相交的另外两个点
					XMVECTOR v0v1Vec = triangleList[triIdx].point[1] - triangleList[triIdx].point[0];
					XMVECTOR v0v2Vec = triangleList[triIdx].point[2] - triangleList[triIdx].point[0];

					float hitPointRatio = edge - XMVectorGetByIndex(triangleList[triIdx].point[0], component);
					float distAlong_v0v1 = hitPointRatio / XMVectorGetByIndex(v0v1Vec, component);
					float distAlong_v0v2 = hitPointRatio / XMVectorGetByIndex(v0v2Vec, component);
					v0v1Vec = distAlong_v0v1 * v0v1Vec + triangleList[triIdx].point[0];
					v0v2Vec = distAlong_v0v2 * v0v2Vec + triangleList[triIdx].point[0];

					triangleList[triIdx].point[1] = v0v2Vec;
                    triangleList[triIdx].point[2] = v0v1Vec;
				}
				else if (insideVertexCount == 2)
				{
					// 裁剪后需要分开成两个三角形

					// 把当前三角形后面的三角形(如果存在的话)复制出来，这样
					// 我们就可以用算出来的新三角形覆盖它
					triangleList[numTriangles] = triangleList[triIdx + 1];
					triangleList[triIdx + 1].isCulled = false;

					// 找出三角形与当前平面相交的另外两个点
					XMVECTOR v2v0Vec = triangleList[triIdx].point[0] - triangleList[triIdx].point[2];
					XMVECTOR v2v1Vec = triangleList[triIdx].point[1] - triangleList[triIdx].point[2];

					float hitPointRatio = edge - XMVectorGetByIndex(triangleList[triIdx].point[2], component);
					float distAlong_v2v0 = hitPointRatio / XMVectorGetByIndex(v2v0Vec, component);
					float distAlong_v2v1 = hitPointRatio / XMVectorGetByIndex(v2v1Vec, component);
					v2v0Vec = distAlong_v2v0 * v2v0Vec + triangleList[triIdx].point[2];
					v2v1Vec = distAlong_v2v1 * v2v1Vec + triangleList[triIdx].point[2];

					// 添加三角形
					triangleList[triIdx + 1].point[0] = triangleList[triIdx].point[0];
					triangleList[triIdx + 1].point[1] = triangleList[triIdx].point[1];
					triangleList[triIdx + 1].point[2] = v2v0Vec;

					triangleList[triIdx].point[0] = triangleList[triIdx + 1].point[1];
					triangleList[triIdx].point[1] = triangleList[triIdx + 1].point[2];
					triangleList[triIdx].point[2] = v2v1Vec;

					// 添加三角形数目，跳过我们刚插入的三角形
					++numTriangles;
					++triIdx;
				}
			}
		}
		for (int triIdx = 0; triIdx < numTriangles; ++triIdx)
		{
			if (!triangleList[triIdx].isCulled)
			{
				for (int vtxIdx = 0; vtxIdx < 3; ++vtxIdx)
				{
					float z = XMVectorGetZ(triangleList[triIdx].point[vtxIdx]);
					outNearPlane = (std::min)(outNearPlane, z);
					outFarPlane = (std::max)(outFarPlane, z);
				}
			}
		}
	}
}
