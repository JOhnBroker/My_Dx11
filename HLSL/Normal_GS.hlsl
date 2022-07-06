#include "Basic_GS.hlsli"

[maxvertexcount(2)]
void GS(point VertexPosHWNormalColor input[1],inout LineStream<VertexPosHWNormalColor> output)
{
    matrix viewProj = mul(g_View, g_Proj);
    
    VertexPosHWNormalColor v;
    
    v.posW = input[0].posW + input[0].normalW * 0.1f;
    v.posH = mul(float4(v.posW, 1.0f), viewProj);
    v.normalW = input[0].normalW;
    v.color = input[0].color;
    
    output.Append(v);
    
    v.posW = input[0].posW + input[0].normalW * 0.5f;
    v.posH = mul(float4(v.posW, 1.0f), viewProj);
    
    output.Append(v);
}