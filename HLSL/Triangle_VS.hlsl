#include "Basic_GS.hlsli"

VertexPosHColor VS(VertexPosColor vIn)
{
    matrix worldViewProj = mul(mul(g_World, g_View), g_Proj);
    VertexPosHColor vOut;
    vOut.color = vIn.color;
    vOut.posH = mul(float4(vIn.posL, 1.0f), worldViewProj);
    return vOut;
}