#ifndef FORWARD_HLSL
#define FORWARD_HLSL

#include "Rendering.hlsl"

// ������Դ��ɫ
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

// ֻ����alpha���ԣ�����ɫ������pre-z pass
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

#endif