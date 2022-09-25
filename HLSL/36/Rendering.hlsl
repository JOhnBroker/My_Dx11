#ifndef RENDERING_HLSL
#define RENDERING_HLSL

#include "FullScreenTriangle.hlsl"
#include "ConstantBuffers.hlsl"
#include "CascadedShadow.hlsl"

// 几何阶段

Texture2D g_TextureDiffuse : register(t0);
Texture2D<float> g_TexturePercentLit : register(t1);
Texture2D g_TextureCascadeColor : register(t2);
SamplerState g_SamplerDiffuse : register(s0);

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 texCoord : TEXCOORD0;
    float4 shadowPosV : TEXCOORD1;
    float depthV : TEXCOORD2;
};

VertexOut GeometryVS(VertexPosNormalTex input)
{
    VertexOut output;
    
    output.posH = mul(float4(input.posL, 1.0f), g_WorldViewProj);
    output.posW = mul(float4(input.posL, 1.0f), g_World).xyz;
    output.normalW = mul(float4(input.normalL, 0.0f), g_WorldInvTranspose).xyz;
    output.texCoord = input.texCoord;
    output.shadowPosV = mul(float4(output.posW,1.0f), g_ShadowView);
    output.depthV = mul(float4(input.posL, 1.0f), g_WorldView).z;
    
    return output;
}

//--------------------------------------------------------------------------------------
// 延迟阴影阶段
//--------------------------------------------------------------------------------------
void DeferredShadowWithColorPS(VertexOut input, 
                                out float percentLit : SV_Target0, 
                                out float4 visualizeCascadeColor : SV_Target1)
{
    int cascadeIndex = 0;
    int nextCascadeIndex = 0;
    float blendAmount = 0.0f;
    percentLit = CalculateCascadedShadow(input.shadowPosV, input.depthV,
                                        cascadeIndex, nextCascadeIndex, blendAmount);
    visualizeCascadeColor = GetCascadeColorMultipler(cascadeIndex, nextCascadeIndex, saturate(blendAmount));
}

float DeferredShadowPS(VertexOut input) : SV_Target
{
    int cascadeIndex = 0;
    int nextCascadeIndex = 0;
    float blendAmount = 0.0f;
    float percentLit = CalculateCascadedShadow(input.shadowPosV, input.depthV,
                                        cascadeIndex, nextCascadeIndex, blendAmount);
    return percentLit;
}

// 光照阶段

float4 ForwardPS(VertexOut input) : SV_Target
{
    int cascadeIndex = 0;
    int nextCascadeIndex = 0;
    float blendAmount = 0.0f;
    float percentLit = CalculateCascadedShadow(input.shadowPosV, input.depthV,
        cascadeIndex, nextCascadeIndex, blendAmount);
    float4 diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    uint texWidth = 0, texHeight = 0;
    g_TextureDiffuse.GetDimensions(texWidth, texHeight);
    if (texWidth > 0 && texHeight > 0)
        diffuse = g_TextureDiffuse.Sample(g_SamplerDiffuse, input.texCoord);
    
    float4 visualizeCascadeColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (g_VisualizeCascades)
    {
        visualizeCascadeColor = GetCascadeColorMultipler(cascadeIndex, nextCascadeIndex, saturate(blendAmount));
    }
    
    //
    float3 lightDir[4] =
    {
        float3(-1.0f, 1.0f, -1.0f),
        float3(1.0f, 1.0f, -1.0f),
        float3(0.0f, -1.0f, 0.0f),
        float3(1.0f, 1.0f, 1.0f)
    };
    
    float lighting = saturate(dot(lightDir[0], input.normalW)) * 0.05f +
                     saturate(dot(lightDir[1], input.normalW)) * 0.05f +
                     saturate(dot(lightDir[2], input.normalW)) * 0.05f +
                     saturate(dot(lightDir[3], input.normalW)) * 0.05f;
    float shadowLighting = lighting * 0.5f;
    lighting += saturate(dot(-g_LightDir, input.normalW));
    lighting = lerp(shadowLighting, lighting, percentLit);
    return lighting * visualizeCascadeColor * diffuse;
}

float4 DeferredShadowForwardPS(VertexOut input) : SV_Target
{
    float4 diffuse = g_TextureDiffuse.Sample(g_SamplerDiffuse, input.texCoord);
    float percentLit = g_TexturePercentLit.Load(int3(input.posH.xy, 0));
    float4 visualizeCascadeColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    uint texWidth = 0, texHeight = 0;
    g_TextureCascadeColor.GetDimensions(texWidth, texHeight);
    if (texWidth > 0 && texHeight > 0)
        g_TextureCascadeColor.Load(int3(input.posH.xy, 0));
    
    // 灯光硬编码
    float3 lightDir[4] =
    {
        float3(-1.0f, 1.0f, -1.0f),
        float3(1.0f, 1.0f, -1.0f),
        float3(0.0f, -1.0f, 0.0f),
        float3(1.0f, 1.0f, 1.0f)
    };
    
    // 类似环境光的照明
    float lighting = saturate(dot(lightDir[0],input.normalW))*0.05f+
                     saturate(dot(lightDir[1],input.normalW))*0.05f+
                     saturate(dot(lightDir[2],input.normalW))*0.05f+
                     saturate(dot(lightDir[3],input.normalW))*0.05f;
    float shadowLighting = lighting * 0.5f;
    lighting += saturate(dot(-g_LightDir, input.normalW));
    lighting = lerp(shadowLighting, lighting, percentLit);
    return lighting * visualizeCascadeColor * diffuse;
    
}

#endif