#include "Texture2D.h"

Texture2DBase::Texture2DBase(ID3D11Device* device,
	const CD3D11_TEXTURE2D_DESC& texDesc, const CD3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
	:m_Width(texDesc.Width), m_Height(texDesc.Height)
{
	m_pTexture.Reset();
	m_pTextureSRV.Reset();

	device->CreateTexture2D(&texDesc, nullptr, m_pTexture.GetAddressOf());

	// 用实际产生的mip等级等数据更新
	D3D11_TEXTURE2D_DESC desc;
	m_pTexture->GetDesc(&desc);

	if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)) 
	{
		device->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, m_pTextureSRV.GetAddressOf());
	}

}

void SetDebugObjectName(std::string_view name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	::SetDebugObjectName(m_pTexture.Get(), name);
	::SetDebugObjectName(m_pTextureSRV.Get(), std::string(name) + ".SRV");
#endif
}


Texture2D::Texture2D(ID3D11Device* device, uint32_t width, uint32_t height,
	DXGI_FORMAT format, uint32_t mipLevels, uint32_t bindFlags)
	:Texture2DBase(device,
		CD3D11_TEXTURE2D_DESC(format, width, height, 1, mipLevels, bindFlags),
		CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, format)) 
{
	D3D11_TEXTURE2D_DESC desc;
	m_pTexture->GetDesc(&desc);
	m_MipLevels = desc.MipLevels;
	if (bindFlags & D3D11_BIND_RENDER_TARGET) 
	{
		device->CreateRenderTargetView(m_pTexture.Get(), nullptr, m_pTextureRTV.GetAddressOf());
	}
	if (bindFlags & D3D11_BIND_UNORDERED_ACCESS) 
	{
		device->CreateUnorderedAccessView(m_pTexture.Get(), nullptr, m_pTextureUAV.GetAddressOf());
	}
}

void Texture2D::SetDebugObjectName(std::string_view name)
{
	Texture2DBase::SetDebugObjectName(name);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	::SetDebugObjectName(m_pTextureRTV.Get(), std::string(name) + ".RTV");
#endif
}

Texture2DMS::Texture2DMS(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const DXGI_SAMPLE_DESC& sampleDesc, uint32_t bindFlags)
{
}

void Texture2DMS::SetDebugObjectName(std::string_view name)
{
	Texture2DBase::SetDebugObjectName(name);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	::SetDebugObjectName(m_pTextureRTV.Get(), std::string(name) + ".RTV");
#endif
}

TextureCube::TextureCube(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t mipLevels, uint32_t bindFlags)
{
}

void TextureCube::SetDebugObjectName(std::string_view name)
{
}


Texture2DArray::Texture2DArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t arraySize, uint32_t mipLevels, uint32_t bindFlags)
{
}

void Texture2DArray::SetDebugObjectName(std::string_view name)
{
}

Texture2DMSArray::Texture2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t arraySize, const DXGI_SAMPLE_DESC& sampleDesc, uint32_t bindFlags)
{
}

void Texture2DMSArray::SetDebugObjectName(std::string_view name)
{
}

Depth2D::Depth2D(ID3D11Device* device, uint32_t width, uint32_t height, DepthStencilBitsFlag depthStencilBitsFlag, uint32_t bindFlags)
{
}

void Depth2D::SetDebugObjectName(std::string_view name)
{
}

Depth2DMS::Depth2DMS(ID3D11Device* device, uint32_t width, uint32_t height, const DXGI_SAMPLE_DESC& sampleDesc, DepthStencilBitsFlag depthStencilBitsFlag, uint32_t bindFlags)
{
}

void Depth2DMS::SetDebugObjectName(std::string_view name)
{
}

Depth2DArray::Depth2DArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t arraySize, DepthStencilBitsFlag depthStencilBitsFlag, uint32_t bindFlags)
{
}

void Depth2DArray::SetDebugObjectName(std::string_view name)
{
}

Depth2DMSArray::Depth2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t arraySize, const DXGI_SAMPLE_DESC& sampleDesc, DepthStencilBitsFlag depthStencilBitsFlag, uint32_t bindFlags)
{
}

void Depth2DMSArray::SetDebugObjectName(std::string_view name)
{
	Texture2DBase::SetDebugObjectName(name);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	for (size_t i = 0; i < m_pDepthStencilElements.size(); ++i)
		::SetDebugObjectName(m_pDepthStencilElements[i].Get(), std::string(name) + ".DSV[" + std::to_string(i) + "]");
	for (size_t i = 0; i < m_pShaderResourceElements.size(); ++i)
		::SetDebugObjectName(m_pShaderResourceElements[i].Get(), std::string(name) + ".SRV[" + std::to_string(i) + "]");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

