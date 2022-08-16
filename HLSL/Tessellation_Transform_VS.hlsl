#include "Tessellation.hlsli"

float4 VS(float3 posL : POSITION) : SV_Position
{
    return mul(float4(posL, 1.0f), g_WorldViewProj);

}
