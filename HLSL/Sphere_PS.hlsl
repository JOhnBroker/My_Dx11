#include "Basic_GS.hlsli"

float4 PS(VertexPosHWNormalColor pIn):SV_Target
{
    
    pIn.normalW = normalize(pIn.normalW);
    
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    ComputeDirectionalLight(g_Material, g_DirLight[0], pIn.normalW, toEyeW, ambient, diffuse, spec);
    
    float4 color = pIn.color * (ambient + diffuse) + spec;

    return color;
}