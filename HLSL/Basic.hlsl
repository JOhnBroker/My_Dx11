#ifndef BASIC_HLSL
#define BASIC_HLSL

#include "LightHelper.hlsl"

Texture2D g_DiffuseMap : register(t0);  // ��������
Texture2D g_NormalMap : register(t1);   // ������ͼ
Texture2D g_ShadowMap : register(t2);   // ��Ӱ��ͼ
SamplerState g_Sam : register(s0); // ���Թ���+Wrap������
SamplerState g_SamShadow : register(s1); // �����+Clamp������

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
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
#if defined USE_NOEMAL_MAP
    float4 tangentL :TANGENT;
#endif
    float2 tex : TEXCOORD;
};

struct VertexOutput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // �������е�λ��
    float3 normalW : NORMAL; // �������������еķ���
#if defined USE_NOEMAL_MAP
    float4 tangentW :TANGENT;
#endif
    float2 tex : TEXCOORD;
    float4 ShadowPosH : TEXCOORD1;
};

VertexOutput BasicVS(VertexInput vIn)
{
    VertexOutput vOut;
    vector posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posW = posW.xyz;
    vOut.posH = mul(posW, g_ViewProj);
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
#if defined USE_NOEMAL_MAP
    vOut.tangentW = mul(vIn.tangentL, g_World);
#endif
    vOut.tex = vIn.tex;
    vOut.ShadowPosH = mul(posW, g_ShadowTransform);
    return vOut;
}

// ������ɫ��
float4 PS(VertexOutput pIn) : SV_Target
{
    uint texWidth, texHeight;
    g_DiffuseMap.GetDimensions(texWidth, texHeight);
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    [flatten]
    if (texWidth > 0 && texHeight > 0)
    {
        // ��ǰ����Alpha�ü����Բ�����Ҫ������ؿ��Ա����������
        texColor = g_DiffuseMap.Sample(g_Sam, pIn.tex);
        clip(texColor.a - 0.1f);
    }

    // ��׼��������
    pIn.normalW = normalize(pIn.normalW);

    // �������ָ���۾����������Լ��������۾��ľ���
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);
    float distToEye = distance(g_EyePosW, pIn.posW);
    
#if defined USE_NOEMAL_MAP
    pIn.tangentW = normalize(pIn.tangentW);
    float3 normalMapSample = g_NormalMap.Sample(g_Sam, pIn.tex).rgb;
    pIn.normalW = NormalSampleToWorldSpace(normalMapSample, pIn.normalW, pIn.tangentW);
#endif
    
    // ��ʼ��Ϊ0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i = 0;
    
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
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