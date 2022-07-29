#include "SkyBox.hlsli"

[maxvertexcount(18)]
void GS(
	triangle VertexPos input[3],
	inout TriangleStream<VertexPosLRT> output
)
{
    [unroll]
    for (int i = 0; i < 6; ++i)
    {
        VertexPosLRT vertex;
        // 指定该三角形到第i个渲染目标
        vertex.RTIndex = i;
        
        [unroll]
        for (int j = 0; j < 3; ++j)
        {
            //vertex.PosH = mul(float4(input[j].PosL, 1.0f), g_WorldViewProj).xyww;
            float4 posH = mul(float4(input[j].PosL, 1.0f), g_Views[i]);
            vertex.PosH = posH.xyww;
            vertex.PosL = input[j].PosL;
            
            output.Append(vertex);
        }
        output.RestartStrip();
    }
}