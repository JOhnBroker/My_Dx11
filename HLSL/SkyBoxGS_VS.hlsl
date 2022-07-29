#include "SkyBox.hlsli"

VertexPosL VS(VertexPos vIn)
{
    VertexPosL vOut;
    vOut.PosH = float4(vIn.PosL, 1.0f);
    vOut.PosL = vIn.PosL;
    return vOut;
}