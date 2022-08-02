#include "LightHelper.hlsli"

Texture2D g_DiffuseMap : register(t0);
Texture2D g_NormalMap : register(t1);
TextureCube g_TexCube : register(t2);
SamplerState g_Sam : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
}

cbuffer CBChangesEveryObjectDrawing : register(b1)
{
    Material g_Material;
}

cbuffer CBDrawingStates : register(b2)
{
    int g_ReflectionEnabled;
    int g_RefractionEnabled;
    float g_Eta; // 空气/介质折射比
    float g_Pad;
}

cbuffer CBChangesEveryFrame : register(b3)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
    float g_Pad2;
}

cbuffer CBChangesRarely : register(b4)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTexRT
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float2 tex : TEXCOORD;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

struct VertexPosHWNormalColorTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

struct VertexPosNormalTangentTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float4 tangentL : TANGENT;
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTangentTex
{
    float4 posH : SV_Position;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float4 tangentW : TANGENT;
    float2 tex : TEXCOORD;
};

// 实例
struct InstancePosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
    matrix world : World;
    matrix worldInvTranspose : WorldInvTranspose;
};

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float4 tangentW)
{
    // 将读取到法向量中的每个分量从[0, 1]还原到[-1, 1]
    float3 normalT = 2.0f * normalMapSample - 1.0f;
    
    // 构建位于世界坐标系的切线空间
    float3 N = unitNormalW;
    float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N); // 施密特正交化
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    
     // 将凹凸法向量从切线空间变换到世界坐标系
    float3 bumpedNormalW = mul(normalT, TBN);
    return bumpedNormalW;

}