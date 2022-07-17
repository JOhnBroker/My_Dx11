#include <algorithm>
#include <unordered_map>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <filesystem>
#include "XUtil.h"
#include <d3d11_1.h>
#include "EffectHelper.h"

using namespace Microsoft::WRL;

# pragma warning(disable: 26812)



//
// 代码宏
//

#define EFFECTHELPER_CREATE_SHADER(FullShaderType, ShaderType)\
{\
    m_##FullShaderType##s[nameID] = std::make_shared<FullShaderType##Info>();\
    m_##FullShaderType##s[nameID]->name = name;\
    hr = device->Create##FullShaderType(blob->GetBufferPointer(), blob->GetBufferSize(),\
        nullptr, m_##FullShaderType##s[nameID]->p##ShaderType.GetAddressOf());\
    break;\
}

#define EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(FullShaderType, ShaderType) \
{\
    if (!pDesc->name##ShaderType.empty())\
    {\
        auto it = pImpl->m_##FullShaderType##s.find(StringToID(pDesc->name##ShaderType));\
        if (it != pImpl->m_##FullShaderType##s.end())\
        {\
            pEffectPass->p##ShaderType##Info = it->second;\
            auto& pCBData = it->second->pParamData;\
            if (pCBData)\
            {\
                pEffectPass->p##ShaderType##ParamData = std::make_unique<CBufferData>(pCBData->cbufferName.c_str(), pCBData->startSlot, (uint32_t)pCBData->cbufferData.size()); \
                it->second->pParamData->CreateBuffer(device);\
            }\
        }\
        else\
            return E_INVALIDARG;\
    }\
}

#define EFFECTHELPER_SET_SHADER_DEBUG_NAME(FullShaderType, ShaderType) \
{\
    for (auto& it : pImpl->m_##FullShaderType##s)\
    {\
        std::string name##ShaderType = std::string(name) + "." + it.second->name;\
        it.second->p##ShaderType->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32_t)name##ShaderType.size(), name##ShaderType.c_str());\
    }\
}

#define EFFECTPASS_SET_SHADER(ShaderType)\
{\
    deviceContext->ShaderType##SetShader(p##ShaderType##Info->p##ShaderType.Get(), nullptr, 0);\
}

#define EFFECTPASS_SET_CONSTANTBUFFER(ShaderType)\
{\
    uint32_t slot = 0, mask = p##ShaderType##Info->cbUseMask;\
    while (mask) {\
        if ((mask & 1) == 0) {\
            ++slot, mask >>= 1;\
            continue;\
        }\
        uint32_t zero_bit = ((mask + 1) | mask) ^ mask;\
        uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit));\
        if (count == 1) {\
            CBufferData& cbData = cBuffers.at(slot);\
            cbData.UpdateBuffer(deviceContext);\
            deviceContext->ShaderType##SetConstantBuffers(slot, 1, cbData.cBuffer.GetAddressOf());\
            ++slot, mask >>= 1;\
        }\
        else {\
            std::vector<ID3D11Buffer*> constantBuffers(count);\
            for (uint32_t i = 0; i < count; ++i) {\
                CBufferData& cbData = cBuffers.at(slot + i);\
                cbData.UpdateBuffer(deviceContext);\
                constantBuffers[i] = cbData.cBuffer.Get();\
            }\
            deviceContext->ShaderType##SetConstantBuffers(slot, count, constantBuffers.data());\
            slot += count + 1, mask >>= (count + 1);\
        }\
    }\
}\


#define EFFECTPASS_SET_PARAM(ShaderType)\
{\
    if (!p##ShaderType##Info->params.empty())\
    {\
        if (p##ShaderType##ParamData->isDirty)\
        {\
            p##ShaderType##ParamData->isDirty = false;\
            p##ShaderType##Info->pParamData->isDirty = true;\
            memcpy_s(p##ShaderType##Info->pParamData->cbufferData.data(), p##ShaderType##ParamData->cbufferData.size(),\
                p##ShaderType##ParamData->cbufferData.data(), p##ShaderType##ParamData->cbufferData.size());\
            p##ShaderType##Info->pParamData->UpdateBuffer(deviceContext);\
        }\
        deviceContext->ShaderType##SetConstantBuffers(p##ShaderType##Info->pParamData->startSlot,\
            1, p##ShaderType##Info->pParamData->cBuffer.GetAddressOf());\
    }\
}

#define EFFECTPASS_SET_SAMPLER(ShaderType)\
{\
    uint32_t slot = 0, mask = p##ShaderType##Info->ssUseMask;\
    while (mask) {\
        if ((mask & 1) == 0) {\
            ++slot, mask >>= 1;\
            continue;\
        }\
        uint32_t zero_bit = ((mask + 1) | mask) ^ mask;\
        uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit));\
        if (count == 1) {\
            deviceContext->ShaderType##SetSamplers(slot, 1, samplers.at(slot).pSS.GetAddressOf());\
            ++slot, mask >>= 1;\
        }\
        else {\
            std::vector<ID3D11SamplerState*> samplerStates(count);\
            for (uint32_t i = 0; i < count; ++i)\
                samplerStates[i] = samplers.at(slot + i).pSS.Get();\
            deviceContext->ShaderType##SetSamplers(slot, count, samplerStates.data()); \
            slot += count + 1, mask >>= (count + 1);\
        }\
    }\
}\

#define EFFECTPASS_SET_SHADERRESOURCE(ShaderType)\
{\
    uint32_t slot = 0;\
    for (uint32_t i = 0; i < 4; ++i, slot = i * 32){\
        uint32_t mask = p##ShaderType##Info->srUseMasks[i];\
        while (mask) {\
            if ((mask & 1) == 0) {\
                ++slot, mask >>= 1; \
                continue; \
            }\
            uint32_t zero_bit = ((mask + 1) | mask) ^ mask; \
            uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit)); \
            if (count == 1) {\
                deviceContext->ShaderType##SetShaderResources(slot, 1, shaderResources.at(slot).pSRV.GetAddressOf()); \
                ++slot, mask >>= 1; \
            }\
            else {\
                std::vector<ID3D11ShaderResourceView*> srvs(count); \
                for (uint32_t i = 0; i < count; ++i)\
                    srvs[i] = shaderResources.at(slot + i).pSRV.Get(); \
                deviceContext->ShaderType##SetShaderResources(slot, count, srvs.data()); \
                slot += count + 1, mask >>= (count + 1); \
            }\
        }\
    }\
}\

//
// 枚举与类声明
//

enum ShaderFlag
{
    PixelShader = 0x1,
    VertexShader = 0x2,
    GeometryShader = 0x4,
    HullShader = 0x8,
    DomainShader = 0x10,
    ComputeShader =0x20
};

// 着色器资源
struct ShaderResource
{
    std::string name;
    D3D11_SRV_DIMENSION dim;
    ComPtr<ID3D11ShaderResourceView>pSRV;
};

// 可读写资源
struct RWResource
{
    std::string name;
    D3D11_UAV_DIMENSION dim;
    ComPtr<ID3D11UnorderedAccessView> pUAV;
    uint32_t initialCount;
    bool enableCounter;
    bool firstInit;     // 防止重复清零
};

// 采样器状态
struct SamplerState
{
    std::string name;
    ComPtr<ID3D11SamplerState> pSS;
};

// 内部使用的常量缓冲区数据
struct CBufferData
{
    BOOL isDirty = false;
    ComPtr<ID3D11Buffer> cBuffer;
    std::vector<uint8_t> cbufferData;
    std::string cbufferName;
    uint32_t startSlot = 0;

    CBufferData() = default;
    CBufferData(const std::string& name, uint32_t startSlot, uint32_t byteWidth, BYTE* initData = nullptr) :
        cbufferData(byteWidth), cbufferName(name), startSlot(startSlot) 
    {
        if (initData) 
        {
            memcpy_s(cbufferData.data(), byteWidth, initData, byteWidth);
        }
    }

    HRESULT CreateBuffer(ID3D11Device* device) 
    {
        if(cBuffer != nullptr)
        {
            return S_OK;
        }
        D3D11_BUFFER_DESC cbd;
        ZeroMemory(&cbd, sizeof(cbd));
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.ByteWidth = (uint32_t)cbufferData.size();
        return device->CreateBuffer(&cbd, nullptr, cBuffer.GetAddressOf());
    }

    void UpdateBuffer(ID3D11DeviceContext* deviceContext) 
    {
        if (isDirty) 
        {
            isDirty = false;
            D3D11_MAPPED_SUBRESOURCE mappedData;
            deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
            memcpy_s(mappedData.pData, cbufferData.size(), cbufferData.data(), cbufferData.size());
            deviceContext->Unmap(cBuffer.Get(), 0);
        }
    }
    void BindVS(ID3D11DeviceContext* devieContext) 
    {
        devieContext->VSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
    }

	void BindHS(ID3D11DeviceContext* devieContext)
	{
		devieContext->HSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindDS(ID3D11DeviceContext* devieContext)
	{
		devieContext->DSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindGS(ID3D11DeviceContext* devieContext)
	{
		devieContext->GSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindCS(ID3D11DeviceContext* devieContext)
	{
		devieContext->CSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}
    
    void BindPS(ID3D11DeviceContext* devieContext)
	{
		devieContext->PSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}
};

struct ConstantBufferVariable :public IEffectConstantBufferVariable
{
    ConstantBufferVariable() = default;
    ~ConstantBufferVariable() override {}
    ConstantBufferVariable(std::string_view name_, uint32_t offset, uint32_t size, CBufferData* pData)
        :name(name_), startByteOffset(offset), byteWidth(size), pCBufferData(pData) 
    {
    }

    void SetUInt(uint32_t val) override 
    {
        SetRaw(&val, 0, 4);
    }

    void SetSInt(int val) override 
    {
        SetRaw(&val, 0, 4);
    }

    void SetFloat(float val) override 
    {
        SetRaw(&val, 0, 4);
    }

    void SetUIntVector(uint32_t numComponents, const uint32_t data[4]) override 
    {
        if (numComponents > 4) 
        {
            numComponents = 4;
        }
        uint32_t byteCount = numComponents * sizeof(uint32_t);
        if (byteCount > byteWidth) 
        {
            byteCount = byteWidth;
        }
        SetRaw(data, 0, byteCount);
    }

	void SetSIntVector(uint32_t numComponents, const int data[4]) override
	{
		if (numComponents > 4)
			numComponents = 4;
		uint32_t byteCount = numComponents * sizeof(int);
		if (byteCount > byteWidth)
			byteCount = byteWidth;
		SetRaw(data, 0, byteCount);
	}

	void SetFloatVector(uint32_t numComponents, const float data[4]) override
	{
		if (numComponents > 4)
			numComponents = 4;
		uint32_t byteCount = numComponents * sizeof(float);
		if (byteCount > byteWidth)
			byteCount = byteWidth;
		SetRaw(data, 0, byteCount);
	}

    void SetUIntMatrix(uint32_t rows, uint32_t cols, const uint32_t* noPadData) override 
    {
        SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(noPadData));
    }

	void SetSIntMatrix(uint32_t rows, uint32_t cols, const int* noPadData) override
	{
		SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(noPadData));
	}

	void SetFloatMatrix(uint32_t rows, uint32_t cols, const float* noPadData) override
	{
		SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(noPadData));
	}

    void SetRaw(const void* data, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF)override
    {
        if (!data || byteOffset > byteWidth) 
        {
            return;
        }
        if (byteOffset + byteCount > byteWidth) 
        {
            byteCount = byteWidth - byteOffset;
        }
        // 
        if (memcmp(pCBufferData->cbufferData.data() + startByteOffset + byteOffset, data, byteCount)) 
        {
            memcpy_s(pCBufferData->cbufferData.data() + startByteOffset + byteOffset, byteCount, data, byteCount);
            pCBufferData->isDirty = true;
        }
    }

    struct PropertyFunctor 
    {
        PropertyFunctor(ConstantBufferVariable& _cbv) :cbv(_cbv) {}
        void operator()(int val) { cbv.SetSInt(val); }
        void operator()(uint32_t val) { cbv.SetUInt(val); }
        void operator()(float val) { cbv.SetFloat(val); }
        void operator()(const DirectX::XMFLOAT2& val) { cbv.SetFloatVector(2,reinterpret_cast<const float*>(&val)); }
        void operator()(const DirectX::XMFLOAT3& val) { cbv.SetFloatVector(3,reinterpret_cast<const float*>(&val)); }
        void operator()(const DirectX::XMFLOAT4& val) { cbv.SetFloatVector(4,reinterpret_cast<const float*>(&val)); }
        void operator()(const DirectX::XMFLOAT4X4& val) { cbv.SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&val)); }
        void operator()(const std::vector<float>& val) { cbv.SetRaw(val.data()); }
        void operator()(const std::vector<DirectX::XMFLOAT4>& val) { cbv.SetRaw(val.data()); }
        void operator()(const std::vector<DirectX::XMFLOAT4X4>& val) { cbv.SetRaw(val.data()); }
        void operator()(const std::string& val) {}
        ConstantBufferVariable& cbv;
    };

    void Set(const Property& prop)override
    {
        std::visit(PropertyFunctor(*this), prop);
    }

    HRESULT GetRaw(void* pOutput, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF)override 
    {
        if (byteOffset > byteWidth || byteCount > byteWidth - byteOffset) 
        {
            return E_BOUNDS;
        }
        if (!pOutput) 
        {
            return E_INVALIDARG;
        }
        memcpy_s(pOutput, byteCount, pCBufferData->cbufferData.data() + startByteOffset + byteOffset, byteCount);
        return S_OK;
    }

	void SetMatrixInBytes(uint32_t rows, uint32_t cols, const BYTE* noPadData)
	{
		// 仅允许1x1到4x4
		if (rows == 0 || rows > 4 || cols == 0 || cols > 4)
			return;
		uint32_t remainBytes = byteWidth < 64 ? byteWidth : 64;
		BYTE* pData = pCBufferData->cbufferData.data() + startByteOffset;
		while (remainBytes > 0 && rows > 0)
		{
			uint32_t rowPitch = sizeof(uint32_t) * cols < remainBytes ? sizeof(uint32_t) * cols : remainBytes;
			// 仅当值不同时更新
			if (memcmp(pData, noPadData, rowPitch))
			{
				memcpy_s(pData, rowPitch, noPadData, rowPitch);
				pCBufferData->isDirty = true;
			}
			noPadData += cols * sizeof(uint32_t);
			pData += 16;
			remainBytes = remainBytes < 16 ? 0 : remainBytes - 16;
		}
	}

	std::string name;
	uint32_t startByteOffset = 0;
	uint32_t byteWidth = 0;
	CBufferData* pCBufferData = nullptr;
};

struct VertexShaderInfo
{
	std::string name;
	ComPtr<ID3D11VertexShader> pVS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t unused = 0;
	uint32_t srUseMasks[4] = {};
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};

struct DomainShaderInfo
{
	std::string name;
	ComPtr<ID3D11DomainShader> pDS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t unused = 0;
	uint32_t srUseMasks[4] = {};
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};

struct HullShaderInfo
{
	std::string name;
	ComPtr<ID3D11HullShader> pHS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t unused = 0;
	uint32_t srUseMasks[4] = {};
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};

struct GeometryShaderInfo
{
	std::string name;
	ComPtr<ID3D11GeometryShader> pGS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t unused = 0;
	uint32_t srUseMasks[4] = {};
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};

struct PixelShaderInfo
{
	std::string name;
	ComPtr<ID3D11PixelShader> pPS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t rwUseMask = 0;
	uint32_t srUseMasks[4] = {};
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};

struct ComputeShaderInfo
{
	std::string name;
	ComPtr<ID3D11ComputeShader> pCS;
	uint32_t cbUseMask = 0;
	uint32_t ssUseMask = 0;
	uint32_t rwUseMask = 0;
	uint32_t srUseMasks[4] = {};
	uint32_t threadGroupSizeX = 0;
	uint32_t threadGroupSizeY = 0;
	uint32_t threadGroupSizeZ = 0;
	std::unique_ptr<CBufferData> pParamData = nullptr;
	std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
};