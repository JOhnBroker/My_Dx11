#include "LightHelper.hlsli"

TextureCube g_TexCube : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CBChangesEveryFrame : register(b0)
{
    matrix g_WorldViewProj;
    matrix g_ViewProjs[6];
}

struct VertexPos
{
    float3 PosL : POSITION;
};

struct VertexPosL
{
    float4 PosH : SV_Position;
    float3 PosL : POSITION;
};

struct VertexPosLRT
{
    float4 PosH : SV_Position;
    float3 PosL : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};