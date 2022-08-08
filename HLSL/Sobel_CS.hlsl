#include "Blur.hlsli"

float3 RGB2Gray(float3 color)
{
    return dot(color, float3(0.212671f, 0.715160f, 0.072169f));
}

[numthreads(16, 16, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    float4 colors[3][3];
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        [unroll]
        for (int j = 0; j < 3; ++j)
        {
            int2 xy = DTid.xy + int2(-1 + j, -1 + i);
            colors[i][j] = g_Input[xy];
        }
    }
    
    // 针对每个颜色通道，利用索贝尔算子估算出关于x的偏导数近似值
    float4 Gx = -1.0f * colors[0][0] - 2.0f * colors[1][0] - 1.0f * colors[2][0] +
        1.0f * colors[0][2] + 2.0f * colors[1][2] + 1.0f * colors[2][2];

    // 针对每个颜色通道，利用索贝尔算子估算出关于x的偏导数近似值
    float4 Gy = -1.0f * colors[2][0] - 2.0f * colors[2][1] - 1.0f * colors[2][2] +
        1.0f * colors[0][0] + 2.0f * colors[0][1] + 1.0f * colors[0][2];
    
    // 梯度向量即为(Gx, Gy)。针对每个颜色通道，计算出梯度大小（即梯度的模拟）
    // 以找到最大的变化率
    float4 mag = sqrt(Gx * Gx + Gy * Gy);
    
    mag = 1.0f - float4(saturate(RGB2Gray(mag.xyz)), 0.0f);
    
    g_Output[DTid.xy] = mag;
}