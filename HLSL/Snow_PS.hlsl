#include "Basic_GS.hlsli"

float4 PS(VertexPosHColor pIn) : SV_Target
{
    return pIn.color;
}