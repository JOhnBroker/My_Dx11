#include "Basic.hlsli"

VertexPosHWNormalColorTex VS(InstancePosNormalTex vIn )
{
    VertexPosHWNormalColorTex vOut;
    
    vector posW = mul(float4(vIn.posL, 1.0f), vIn.world);
    
    vOut.posH = mul(posW, g_ViewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) vIn.worldInvTranspose);
    vOut.tex = vIn.tex;
    vOut.color = vIn.color;
    
	return vOut;
}