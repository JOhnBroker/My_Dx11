#include "Basic_GS.hlsli"

[maxvertexcount(9)]
void GS(triangle VertexPosHColor input[3], inout TriangleStream<VertexPosHColor> output)
{
    // 将一个三角形分裂成三个三角形，即没有v3v4v5的三角形
    //       v1
    //       /\
    //      /  \
    //   v3/____\v4
    //    /\xxxx/\
    //   /  \xx/  \
    //  /____\/____\
    // v0    v5    v2
    VertexPosHColor vertex[6];
    int i;
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        vertex[i] = input[i];
        vertex[i + 3].color = (input[i].color + input[(i + 1) % 3].color) / 2.0f;
        vertex[i + 3].posH = (input[i].posH + input[(i + 1) % 3].posH) / 2.0f;
    }
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        output.Append(vertex[i]);
        output.Append(vertex[3 + i]);
        output.Append(vertex[(i + 2) % 3 + 3]);
        output.RestartStrip();
    }
}