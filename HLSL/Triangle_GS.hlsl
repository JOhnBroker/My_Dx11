#include "Basic_GS.hlsli"

[maxvertexcount(9)]
void GS(triangle VertexPosHColor input[3], inout TriangleStream<VertexPosHColor> output)
{
    // ��һ�������η��ѳ����������Σ���û��v3v4v5��������
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
        vertex[i + 3].Color = (input[i].Color + input[(i + 1) % 3].Color) / 2.0f;
        vertex[i + 3].PosH = (input[i].PosH + input[(i + 1) % 3].PosH) / 2.0f;
    }
    for (i = 0; i < 3; ++i)
    {
        output.Append(vertex[i]);
        output.Append(vertex[3 + i]);
        output.Append(vertex[(i + 2) % 3 + 3]);
        output.RestartStrip();
    }
}