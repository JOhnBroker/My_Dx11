#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

#include "GBuffer.hlsl"

#ifndef MSAA_SAMPLES
#define MSAA_SAMPLES 1
#endif 

//--------------------------------------------------------------------------------------
// ����, ��պе�
// ʹ����պм�������Ⱦ
//--------------------------------------------------------------------------------------
TextureCube<float4> g_SkyboxTexture : register(t5);
Texture2DMS<float, MSAA_SAMPLES> g_DepthTexture : register(t6);
// ������ز����ĳ�����Ⱦ������
Texture2DMS<float4, MSAA_SAMPLES> g_LitTexture : register(t7);

struct SkyboxVSOut
{
    float4 posViewport : SV_Position;
    float3 skyboxCoord : skyboxCoord;
};

SkyboxVSOut SkyboxVS(VertexPosNormalTex input)
{
    SkyboxVSOut output;
    
    // Reversed - Z ,���������Ϊ0.0fΪ������
    output.posViewport = mul(float4(input.posL, 0.0f), g_ViewProj).xyww;
    output.skyboxCoord = input.posL;
    
    return output;
}

float4 SkyboxPS(SkyboxVSOut input) : SV_Target
{
    uint2 coords = input.posViewport.xy;
    
    float3 lit = float3(0.0f, 0.0f, 0.0f);
    float skyboxSamples = 0.0f;
#if MSAA_SAMPLES <=1
    [unroll]
#endif
    for (unsigned int sampleIndex = 0; sampleIndex < MSAA_SAMPLES; ++sampleIndex)
    {
        float depth = g_DepthTexture.Load(coords, sampleIndex);
        
        if (depth <= 0.0f && !g_VisualizeLightCount)
        {
            ++skyboxSamples;
        }
        else
        {
            lit += g_LitTexture.Load(coords, sampleIndex).xyz;
        }
    }
    
    [branch]
    if (skyboxSamples > 0)
    {
        float3 skybox = g_SkyboxTexture.Sample(g_Sam, input.skyboxCoord).xyz;
        lit += skyboxSamples * skybox;
    }
    return float4(lit * rcp((float) MSAA_SAMPLES), 1.0f);
}

#endif