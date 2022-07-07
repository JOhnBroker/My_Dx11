#include "Basic_GS.hlsli"

VertexPosHColor VS(VertexPosColor vIn)
{
    matrix worldViewProj = mul(mul(g_World, g_View), g_Proj);
    VertexPosHColor vOut;
    vOut.posH = mul(float4(vIn.posL, 1.0f), worldViewProj);
    vOut.color = vIn.color;
    return vOut;
}