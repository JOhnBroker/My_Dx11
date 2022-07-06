#include "Basic_GS.hlsli"

[maxvertexcount(6)]
void GS(line VertexPosHWNormalColor input[2], inout TriangleStream<VertexPosHWNormalColor> output)
{
    float3 upDir = normalize(cross(input[0].normalW, (input[1].posW - input[0].posW)));
    VertexPosHWNormalColor v2, v3;
    
    matrix viewProj = mul(g_View, g_Proj);
    
    v2.posW = input[1].posW + upDir * g_CylinderHeight;
    v2.posH = mul(float4(v2.posW, 1.0f), viewProj);
    v2.normalW = input[1].normalW;
    v2.color = input[1].color;

    v3.posW = input[0].posW + upDir * g_CylinderHeight;
    v3.posH = mul(float4(v3.posW, 1.0f), viewProj);
    v3.normalW = input[0].normalW;
    v3.color = input[0].color;
    
    output.Append(input[0]);
    output.Append(input[1]);
    output.Append(v2);
    output.RestartStrip();
    
    output.Append(v2);
    output.Append(v3);
    output.Append(input[0]);    
}