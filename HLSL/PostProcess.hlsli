Texture2D g_Tex : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CB : register(b0)
{
    float g_VisibleRange;
    float3 g_EyePosW;
    float4 g_RectW;
};

struct VertexPosTex
{
    float3 posL : POSITION;
    float2 tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 posH : SV_Position;
    float2 tex : TEXCOORD;
};