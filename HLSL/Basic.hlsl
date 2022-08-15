#ifndef BASIC_HLSL
#define BASIC_HLSL

#include "LightHelper.hlsl"

Texture2D g_DiffuseMap : register(t0);  // 物体纹理
Texture2D g_NormalMap : register(t1);   // 法线贴图
Texture2D g_ShadowMap : register(t2);   // 阴影贴图
Texture2D g_AmbientOcclusionMap : register(t3);
SamplerState g_Sam : register(s0); // 线性过滤+Wrap采样器
SamplerComparisonState g_SamShadow : register(s1); // 点过滤+Clamp采样器

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    matrix g_WorldViewProj;
}

cbuffer CBChangesEveryObjectDrawing : register(b1)
{
    Material g_Material;
}

cbuffer CBChangesEveryFrame : register(b2)
{
    matrix g_ViewProj;
    matrix g_ShadowTransform; // ShadowView * ShadowProj * T
    float3 g_EyePosW;
    float g_Pad;
}

cbuffer CBDrawingStates : register(b3)
{    
    float g_DepthBias; 
}

cbuffer CBChangesRarely : register(b4)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

struct VertexInput
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
#if defined USE_NORMAL_MAP
    float4 tangentL :TANGENT;
#endif
    float2 tex : TEXCOORD;
};

struct VertexOutput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
#if defined USE_NORMAL_MAP
    float4 tangentW :TANGENT;
#endif
    float2 tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
#if defined USE_SSAO_MAP
    float4 SSAOPosH : TEXCOORD2;
#endif
};

VertexOutput BasicVS(VertexInput vIn)
{
    VertexOutput vOut;
    vector posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posW = posW.xyz;
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
#if defined USE_NORMAL_MAP
    vOut.tangentW = float4(mul(vIn.tangentL.xyz, (float3x3) g_World), vIn.tangentL.w);
#endif
    vOut.tex = vIn.tex;
    vOut.ShadowPosH = mul(posW, g_ShadowTransform);
#if defined USE_SSAO_MAP
    // 从NDC坐标[-1, 1]^2变换到纹理空间坐标[0, 1]^2
    // u = 0.5x + 0.5
    // v = -0.5y + 0.5
    // ((xw, yw, zw, w) + (w, -w, 0, 0)) * (0.5, -0.5, 1, 1) = ((0.5x + 0.5)w, (-0.5y + 0.5)w, zw, w)
    //                                                      = (uw, vw, zw, w)
    //                                                      =>  (u, v, z, 1)
    vOut.SSAOPosH = (vOut.posH + float4(vOut.posH.w, -vOut.posH.w, 0.0f, 0.0f)) * float4(0.5f, -0.5f, 1.0f, 1.0f);
#endif
    return vOut;
}

// 像素着色器
float4 BasicPS(VertexOutput pIn) : SV_Target
{
    //uint texWidth, texHeight;
    //g_DiffuseMap.GetDimensions(texWidth, texHeight);
    //float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    //
    //[flatten]
    //if (texWidth > 0 && texHeight > 0)
    //{
    //    // 提前进行Alpha裁剪，对不符合要求的像素可以避免后续运算
    //    texColor = g_DiffuseMap.Sample(g_Sam, pIn.tex);
    //    clip(texColor.a - 0.1f);
    //}
    float4 texColor = g_DiffuseMap.Sample(g_Sam, pIn.tex);
    clip(texColor.a - 0.1f);
    
    // 标准化法向量
    pIn.normalW = normalize(pIn.normalW);

    // 求出顶点指向眼睛的向量，以及顶点与眼睛的距离
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);
    float distToEye = distance(g_EyePosW, pIn.posW);
    
#if defined USE_NORMAL_MAP
    pIn.tangentW = normalize(pIn.tangentW);
    float3 normalMapSample = g_NormalMap.Sample(g_Sam, pIn.tex).rgb;
    pIn.normalW = NormalSampleToWorldSpace(normalMapSample, pIn.normalW, pIn.tangentW);
#endif
    
    float ambientAccess = 1.0f;
    
#if defined USE_SSAO_MAP
    pIn.SSAOPosH /= pIn.SSAOPosH.w;
    ambientAccess = g_AmbientOcclusionMap.SampleLevel(g_Sam, pIn.SSAOPosH.xy, 0.0f).r;
#endif
    
    // 初始化为0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i = 0;
    
    float shadow[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    
    //shadow[0] = CalcShadowFactor(g_Sam, g_ShadowMap, pIn.ShadowPosH, g_DepthBias);
    shadow[0] = CalcShadowFactor(g_SamShadow, g_ShadowMap, pIn.ShadowPosH, g_DepthBias);
    
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += ambientAccess * A;
        diffuse += shadow[i] * D;
        spec += shadow[i] * S;
    }
    
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputePointLight(g_Material, g_PointLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeSpotLight(g_Material, g_SpotLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.diffuse.a;
    return litColor;
}

#endif