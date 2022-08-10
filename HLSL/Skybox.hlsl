#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

uniform matrix g_ViewProj;

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// 后处理, 天空盒等
// 使用天空盒几何体渲染
//--------------------------------------------------------------------------------------
TextureCube<float4> g_SkyboxTexture : register(t5);
Texture2D<float> g_DepthTexture : register(t6);

Texture2D<float4> g_LitTexture : register(t7);

SamplerState g_SamplerDiffuse : register(s0);

struct SkyboxVSOut
{
    float4 posViewport : SV_Position;
    float3 skyboxCoord : skyboxCoord;
};

SkyboxVSOut SkyboxVS(VertexPosNormalTex vIn)
{
    SkyboxVSOut vOut;
    
    vOut.posViewport = mul(float4(vIn.posL, 1.0f), g_ViewProj).xyww;
    vOut.skyboxCoord = vIn.posL;
    return vOut;
}

float4 SkyboxPS(SkyboxVSOut pIn) : SV_Target
{
    uint2 coord = pIn.posViewport.xy;
    
    float3 lit = float3(0.0f, 0.0f, 0.0f);
    float depth = g_DepthTexture[coord];
    if (depth >= 1.0f)
    {
        lit += g_SkyboxTexture.Sample(g_SamplerDiffuse, pIn.skyboxCoord).rgb;
    }
    else
    {
        lit += g_LitTexture[coord].xy;
    }
    
    return float4(lit, 1.0f);
}

#endif