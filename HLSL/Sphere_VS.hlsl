#include "Basic_GS.hlsli"

VertexPosHWNormalColor VS(VertexPosNormalColor vIn)
{
    matrix ViewProj = mul(g_View, g_Proj);
    VertexPosHWNormalColor vOut;
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posH = mul(posW, ViewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3)g_WorldInvTranspose);
    vOut.color = vIn.color;
    
    return vOut;
}