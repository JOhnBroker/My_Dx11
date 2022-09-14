#ifndef FORWARD_HLSL
#define FORWARD_HLSL

#include "Rendering.hlsl"
#include "ForwardTileInfo.hlsl"

// 计算点光源着色
float4 ForwardPS(VertexPosHVNormalVTex input) : SV_Target
{
    uint totalLights, dummy;
    g_light.GetDimensions(totalLights, dummy);
    
    float3 litColor = float3(0.0f, 0.0f, 0.0f);
    
    [branch]
    if (g_VisualizeLightCount)
    {
        litColor = (float(totalLights) * rcp(255.0f)).xxx;
    }
    else
    {
        SurfaceData surface = ComputeSurfaceDataFromGeometry(input);
        for (uint lightIndex = 0; lightIndex < totalLights; ++lightIndex)
        {
            PointLight light = g_light[lightIndex];
            AccumulateColor(surface, light, litColor);
        }
    }
    
    return float4(litColor, 1.0f);
}

// 只进行alpha测试，不上色。用于pre-z pass
void ForwardAlphaTestOnlyPS(VertexPosHVNormalVTex input)
{
    SurfaceData surface = ComputeSurfaceDataFromGeometry(input);
    clip(surface.albedo.a - 0.3f);
}

float4 ForwardAlphaTestPS(VertexPosHVNormalVTex input) : SV_Target
{
    // Always use face normal for alpha tested stuff since it's double-sided
    input.normalV = ComputeFaceNormal(input.posV);
    
    // Clip
    ForwardAlphaTestOnlyPS(input);
    
    // Otherwise run the normal shader
    return ForwardPS(input);
}

StructuredBuffer<TileInfo> g_Tilebuffer : register(t9);

// 计算点光源着色 
float4 ForwardPlusPS(VertexPosHVNormalVTex input) : SV_Target
{
    uint dispatchWidth = (g_FramebufferDimensions.x + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint tilebufferIndex = (uint) input.posH.y / COMPUTE_SHADER_TILE_GROUP_DIM * dispatchWidth +
                            (uint) input.posH.x / COMPUTE_SHADER_TILE_GROUP_DIM;
    float3 litColor = float3(0.0f, 0.0f, 0.0f);
    uint numLights = g_Tilebuffer[tilebufferIndex].tileNumLights;
    [branch]
    if (g_VisualizeLightCount)
    {
        
        const float3 mapTex[] =
        {
            float3(0, 0, 0),
		    float3(0, 0, 1),
		    float3(0, 1, 1),
		    float3(0, 1, 0),
		    float3(1, 1, 0),
		    float3(1, 0, 0),
        };
        const uint mapTexLen = 5;
        const uint maxHeat = 100;
        float p = saturate((float) numLights / maxHeat);
        float l = p * mapTexLen;
        float3 a = mapTex[floor(l)];
        float3 b = mapTex[ceil(l)];
        float4 heatmap = float4(lerp(a, b, l - floor(l)), p);
        litColor = heatmap;
        //litColor = (float(numLights) * rcp(255.0f)).xxx;
    }
    else
    {
        SurfaceData surface = ComputeSurfaceDataFromGeometry(input);
        for (uint lightIndex = 0; lightIndex < numLights; ++lightIndex)
        {
            uint listIndex = g_Tilebuffer[tilebufferIndex].tileLightIndices[lightIndex];
            PointLight light = g_light[listIndex];
            AccumulateColor(surface, light, litColor);
        }
    }
    return float4(litColor, 1.0f);
}

#endif