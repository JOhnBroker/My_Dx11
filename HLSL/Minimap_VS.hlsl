#include "PostProcess.hlsli"

VertexPosHTex VS(uint vertexID : SV_VertexID)
{
    VertexPosHTex vOut;
    float2 grid = float2((vertexID << 1) & 2, vertexID & 2);
    float2 xy = grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    vOut.tex = grid * float2(1.0f, 1.0f);
    vOut.posH = float4(xy, 1.0f, 1.0f);
    return vOut;
}