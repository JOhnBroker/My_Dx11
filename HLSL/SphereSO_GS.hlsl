#include "Basic_GS.hlsli"

[maxvertexcount(12)]
void GS(triangle VertexPosNormalColor input[3], inout TriangleStream<VertexPosNormalColor> output)
{
    //
    // ��һ�������η��ѳ��ĸ������Σ���ͬʱ����v3, v4, v5Ҳ��Ҫ��������
    //       v1
    //       /\
    //      /  \
    //   v3/____\v4
    //    /\xxxx/\
    //   /  \xx/  \
    //  /____\/____\
    // v0    v5    v2
    
    VertexPosNormalColor vertexes[6];

    matrix viewProj = mul(g_View, g_Proj);
    
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        vertexes[i] = input[i];
        vertexes[i + 3].color = lerp(input[i].color, input[(i + 1) % 3].color, 0.5f);
        vertexes[i + 3].normalL = normalize(input[i].normalL + input[(i + 1) % 3].normalL);
        vertexes[i + 3].posL = g_SphereCenter + g_SphereRadius * vertexes[i + 3].normalL;
    }
    output.Append(vertexes[0]);
    output.Append(vertexes[3]);
    output.Append(vertexes[5]);
    output.RestartStrip();

    output.Append(vertexes[3]);
    output.Append(vertexes[4]);
    output.Append(vertexes[5]);
    output.RestartStrip();
    
    output.Append(vertexes[5]);
    output.Append(vertexes[4]);
    output.Append(vertexes[2]);
    output.RestartStrip();
    
    output.Append(vertexes[3]);
    output.Append(vertexes[1]);
    output.Append(vertexes[4]);
    
}