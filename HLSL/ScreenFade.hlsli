Texture2D g_Tex : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CBChangeEveryFrame : register(b0)
{
    float g_FadeAmount;
    float3 g_Pad;
}

cbuffer CBChangeRarely : register(b1)
{
    matrix g_WorldViewProj;
}

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