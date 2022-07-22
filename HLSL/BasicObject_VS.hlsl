#include "Basic.hlsli"

VertexPosHWNormalColorTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalColorTex vOut;
    
    vector posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posH = mul(posW, g_ViewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.tex = vIn.tex;
    vOut.color = g_DiffuseColor;
    
    return vOut;
}