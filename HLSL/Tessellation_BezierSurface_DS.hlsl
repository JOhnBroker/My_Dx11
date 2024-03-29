#include "Tessellation.hlsli"

[domain("quad")]
float4 DS(QuadPatchTess patchTess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<HullOut, 16> bezPatch) : SV_Position
{
    float4 basisU = BernsteinBasis(uv.x);
    float4 basisV = BernsteinBasis(uv.y);
    
    float3 p = CubicBezierSum(bezPatch, basisU, basisV);
    
    float4 posH = mul(float4(p, 1.0f), g_WorldViewProj);
    
    return posH;
}