#include <Waves.h>
#include <ModelManager.h>
#include <TextureManager.h>
#include <DXTrace.h>

using namespace Microsoft::WRL;
using namespace DirectX;

uint32_t Waves::RowCount() const
{
	return m_NumRows;
}

uint32_t Waves::ColumnCount() const
{
	return m_NumCols;
}

void Waves::InitResource(ID3D11Device* device, uint32_t rows, uint32_t cols,
	float texU, float texV, float timeStep, float spatialStep,
	float wavesSpeed, float damping, float flowSpeedX, float flowSpeedY, bool cpuWrite)
{
	m_NumRows = rows;
	m_NumCols = cols;

	m_TexU = texU;
	m_TexV = texV;

	m_TimeStep = timeStep;
	m_SpatialStep = spatialStep;
	m_FlowSpeedX = flowSpeedX;
	m_FlowSpeedY = flowSpeedY;
	m_AccumulateTime = 0.0f;

	float d = damping * timeStep + 2.0f;
	float e = (wavesSpeed * wavesSpeed) * (timeStep * timeStep) / (spatialStep * spatialStep);
	m_K1 = (damping * timeStep - 2.0f) / d;
	m_K2 = (4.0f - 8.0f * e) / d;
	m_K3 = (2.0f * e) / d;


	m_MeshData = Geometry::CreateGrid(XMFLOAT2((cols - 1) * spatialStep, (rows - 1) * spatialStep), XMUINT2(cols - 1, rows - 1), XMFLOAT2(1.0f, 1.0f));
	Model::CreateFromGeometry(m_Model, device, m_MeshData, cpuWrite);
	m_pModel = &m_Model;

	if (TextureManager::Get().GetTexture("Texture\\water2.dds") == nullptr) 
	{
		TextureManager::Get().CreateTexture("Texture\\water2.dds", false, true);
	}
	m_Model.materials[0].Set<std::string>("$Diffuse", "Texture\\water2.dds");
	m_Model.materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	m_Model.materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
	m_Model.materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f));
	m_Model.materials[0].Set<float>("$Opacity", 0.5f);
	m_Model.materials[0].Set<float>("$SpecularPower", 32.0f);
	m_Model.materials[0].Set<XMFLOAT2>("$TexOffset", XMFLOAT2());
	m_Model.materials[0].Set<XMFLOAT2>("$TexScale", XMFLOAT2(m_TexU, m_TexV));
}
