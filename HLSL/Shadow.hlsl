#ifndef SHADOW_HLSL
#define SHADOW_HLSL

#include "FullScreenTriangle.hlsl"

Texture2D g_DiffuseMap : register(t0);
Texture2D g_NormalMap : register(t1);
SamplerState g_Sam : register(s0);

cbuffer CB : register(b0)
{
    matrix g_World;
    matrix g_WorldViewProj;
    matrix g_WorldInvTranspose;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
    float g_HeightScale;
    float g_MaxTessDistance;
    float g_MinTessDistance;
    float g_MaxTessFactor;
    float g_MinTessFactor;
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

struct TessVertexOut
{
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 tex : TEXCOORD;
    float tessFactor : TESS;
};

struct PatchTess
{
    float edgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

struct HullOut
{
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 tex : TEXCOORD;
};

VertexPosHTex ShadowVS(VertexPosNormalTex vIn)
{
    VertexPosHTex vOut;
    
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);
    vOut.texCoord = vIn.texCoord;
    return vOut;
}

TessVertexOut ShadowTessVS(VertexPosNormalTex vIn)
{
    TessVertexOut vOut;
    
    vOut.posW = mul(float4(vIn.posL, 1.0f), g_World).xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.tex = vIn.texCoord;
    
    float d = distance(vOut.posW, g_EyePosW);
    
    float tess = saturate((g_MinTessDistance - d) / (g_MinTessDistance - g_MaxTessDistance));
    
    vOut.tessFactor = g_MinTessFactor + tess * (g_MaxTessFactor - g_MinTessFactor);
    
    return vOut;
}

PatchTess PatchHS(InputPatch<TessVertexOut,3> patch,
                uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    pt.edgeTess[0] = 0.5f * (patch[1].tessFactor + patch[2].tessFactor);
    pt.edgeTess[1] = 0.5f * (patch[2].tessFactor + patch[0].tessFactor);
    pt.edgeTess[2] = 0.5f * (patch[0].tessFactor + patch[1].tessFactor);
    pt.InsideTess = pt.edgeTess[0];
    
    return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut ShadowTessHS(InputPatch<TessVertexOut,3> patch,
            uint i : SV_OutputControlPointID,
            uint patchID : SV_PrimitiveID)
{
    HullOut hOut;
    
    hOut.posW = patch[i].posW;
    hOut.normalW = patch[i].normalW;
    hOut.tex = patch[i].tex;
    
    return hOut;
}

[domain("tri")]
VertexPosHTex ShadowTessDS(PatchTess patchTess,
            float3 bary : SV_DomainLocation,
            const OutputPatch<HullOut, 3> tri)
{
    VertexPosHTex dOut;
    
    float3 posW     = bary.x * tri[0].posW + bary.y * tri[1].posW + bary.z * tri[2].posW;
    float3 normalW  = bary.x * tri[0].normalW + bary.y * tri[1].normalW + bary.z * tri[2].normalW;
    dOut.texCoord   = bary.x * tri[0].tex + bary.y * tri[1].tex + bary.z * tri[2].tex;
    
    normalW = normalize(normalW);
    
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(posW, g_EyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
    
    float h = g_NormalMap.SampleLevel(g_Sam, dOut.texCoord, mipLevel).a;
    
    posW += (g_HeightScale * (h - 1.0f)) * normalW;
    
    dOut.posH = mul(float4(posW, 1.0f), g_ViewProj);
    
    return dOut;
}

// 这仅仅用于Alpha几何裁剪，以保证阴影的显示正确。
// 对于不需要进行纹理采样操作的几何体可以直接将像素
// 着色器设为nullptr
void ShadowPS(VertexPosHTex pIn, uniform float clipValue)
{
    float4 diffuse = g_DiffuseMap.Sample(g_Sam, pIn.texCoord);
    
    // 不要将透明像素写入深度贴图
    clip(diffuse.a - clipValue);
}

float4 DebugShadowPS(VertexPosHTex pIn) : SV_Target
{
    float depth = g_DiffuseMap.Sample(g_Sam, pIn.texCoord).r;
    return float4(depth.rrr, 1.0f);
}

#endif