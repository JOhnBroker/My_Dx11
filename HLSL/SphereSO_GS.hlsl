#include "Basic_GS.hlsli"

[maxvertexcount(12)]
void GS(triangle VertexPosNormalColor input[3], inout TriangleStream<VertexPosNormalColor> output)
{
    //
    // 将一个三角形分裂成四个三角形，但同时顶点v3, v4, v5也需要在球面上
    //       v1
    //       /\
    //      /  \
    //   v3/____\v4
    //    /\xxxx/\
    //   /  \xx/  \
    //  /____\/____\
    // v0    v5    v2
    
    VertexPosNormalColor vertexs[6];
    matrix viewProj = mul(g_View, g_Proj);
    
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        vertexs[i] = input[i];
        vertexs[i + 3].color = lerp(input[i].color, input[(i + 1) % 3].color, 0.5f);
        vertexs[i + 3].normalL = normalize(input[i].normalL + input[(i + 1) % 3].normalL);
        vertexs[i + 3].posL = g_SphereCenter + g_SphereRadius * vertexs[i + 3].normalL;
    }
    output.Append(vertexs[0]);
    output.Append(vertexs[3]);
    output.Append(vertexs[5]);
    output.RestartStrip();

    output.Append(vertexs[3]);
    output.Append(vertexs[4]);
    output.Append(vertexs[5]);
    output.RestartStrip();
    
    output.Append(vertexs[5]);
    output.Append(vertexs[4]);
    output.Append(vertexs[2]);
    output.RestartStrip();
    
    output.Append(vertexs[3]);
    output.Append(vertexs[1]);
    output.Append(vertexs[4]);
    output.RestartStrip();
    
}