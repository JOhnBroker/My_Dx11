#include "Basic.hlsli"

[maxvertexcount(18)]
void GS(
	triangle VertexPosHWNormalTex input[3],
	inout TriangleStream<VertexPosHWNormalTexRT> output
)
{
    [unroll]
    for (int i = 0; i < 6; ++i)
    {
        VertexPosHWNormalTexRT vertex;
        // 指定该三角形到第i个渲染目标
        vertex.RTIndex = i;
        
        [unroll]
        for (int j = 0; j < 3; ++j)
        {
            //vertex.PosH = mul(float4(input[j].PosL, 1.0f), g_ViewProjs[i]).xyww;
            vertex.posH = mul(input[j].posH, g_ViewProjs[i]);
            vertex.posW = input[j].posW;
            vertex.normalW = input[j].normalW;
            vertex.tex = input[j].tex;
            output.Append(vertex);
        }
        output.RestartStrip();
    }
}