#include "SkyBox.hlsli"

float4 PS(VertexPosL pIn) : SV_Target
{
    return g_TexCube.Sample(g_Sam, pIn.PosL);

}