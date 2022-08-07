#include "Waves.hlsli"

[numthreads(16, 16, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    uint x = DTid.x;
    uint y = DTid.y;
    
    g_Output[uint2(x, y)] =
        g_WaveConstant0 * g_PrevSolInput[uint2(x, y)].x +
        g_WaveConstant1 * g_CurrSolInput[uint2(x, y)].x +
        g_WaveConstant2 * (g_CurrSolInput[uint2(x, y + 1)].x +
            g_CurrSolInput[uint2(x, y - 1)].x +
            g_CurrSolInput[uint2(x + 1, y)].x +
            g_CurrSolInput[uint2(x - 1, y)].x);

}