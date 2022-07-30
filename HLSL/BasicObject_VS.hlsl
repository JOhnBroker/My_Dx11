#include "Basic.hlsli"

VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    
    vector posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posH = mul(posW, g_ViewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.tex = vIn.tex;
    return vOut;
}