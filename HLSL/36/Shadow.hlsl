#ifndef SHADOW_HLSL
#define SHADOW_HLSL

#include "FullScreenTriangle.hlsl"

cbuffer CB : register(b0)
{
    matrix g_WorldViewProj;
}

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexPosHTex
{
    float4 posH : SV_Position;
    float2 texCoord : TEXCOORD;
};

VertexPosHTex ShadowVS(VertexPosNormalTex vIn)
{
    VertexPosHTex vOut;
    
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);
    vOut.texCoord = vIn.texCoord;
    
    return vOut;
}

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);

void ShadowPS(VertexPosHTex pIn, uniform float clipValue)
{
    float4 diffuse = g_DiffuseMap.Sample(g_Sam, pIn.texCoord);
    clip(diffuse.a - clipValue);
}

float4 DebugPS(VertexPosHTex pIn) : SV_Target
{
    float depth = g_DiffuseMap.Sample(g_Sam, pIn.texCoord).r;
    return float4(depth.rrr, 1.0f);
}


#endif