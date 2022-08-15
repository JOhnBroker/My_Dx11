#pragma once

#ifndef SSAO_HLSL
#define SSAO_HLSL

#include "FullScreenTriangle.hlsl"

Texture2D g_DiffuseMap : register(t0);
Texture2D g_NormalDepthMap : register(t1);
Texture2D g_RandomVecMap : register(t2);
Texture2D g_InputImage : register(t3);

SamplerState g_SamLinearWrap : register(s0);
SamplerState g_SamLinearClamp : register(s1);
SamplerState g_SamNormalDepth : register(s2);
SamplerState g_SamRandomVec : register(s3);
SamplerState g_SamBlur : register(s4);

cbuffer CBChangesEveryObjectDrawing : register(b0)
{
    // SSAO_NormalDepth 
    matrix g_WorldView;
    matrix g_WorldViewProj;
    matrix g_WorldInvTransposeView;
}

cbuffer CBChangesOnResize : register(b2)
{
    // SSAO
    matrix g_ViewToTexSpace;        // Proj * Texture
    float4 g_FarPlanePoints[3];     // 远平面三角形(覆盖四个角点)，三角形见下
    float2 g_TexelSize;             // (1.0f / W, 1.0f / H)
}

cbuffer CBChangesRarely : register(b3)
{
    float4 g_OffsetVectors[14];
    
    // 观察空间下的坐标
    float g_OcclusionRadius;
    float g_OcclusionFadeStart;
    float g_OcclusionFadeEnd;
    float g_SurfaceEpsilon;
    
    // SSAO_Blur 
    float4 g_BlurWeights[3];
    
    static float s_BlurWeights[12] = (float[12])g_BlurWeights;
   
    int g_BlurRadius;
    float3 g_Pad;
}

// Pass1 几何Buffer生成 (生成法线深度图)

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosHVNormalTex
{
    float4 posH : SV_Position;
    float3 posV : POSITION;
    float3 normalV : NORMAL;
    float2 tex : TEXCOORD0;
};

VertexPosHVNormalTex GeometryVS(VertexPosNormalTex vIn)
{
    VertexPosHVNormalTex vOut;
    
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);
    vOut.posV = mul(float4(vIn.posL, 1.0f), g_WorldView).xyz;
    vOut.normalV = mul(vIn.normalL, (float3x3) g_WorldInvTransposeView);
    vOut.tex = vIn.tex;
    
    return vOut;
}

float4 GeometryPS(VertexPosHVNormalTex pIn, uniform bool alphaClip):SV_Target
{
    pIn.normalV = normalize(pIn.normalV);
    
    if (alphaClip)
    {
        float4 g_TexColor = g_DiffuseMap.Sample(g_SamLinearWrap, pIn.tex);
        clip(g_TexColor.a - 0.1f);
    }
    
    return float4(pIn.normalV, pIn.posV.z);
}

float OcclusionFunction(float distZ)
{
    // 如果depth(q)在depth(p)之后(超出半球范围)，那点q不能遮蔽点p。此外，如果depth(q)和depth(p)过于接近，
    // 我们也认为点q不能遮蔽点p，因为depth(p)-depth(r)需要超过用户假定的Epsilon值才能认为点q可以遮蔽点p
    //
    // 我们用下面的函数来确定遮蔽程度
    //
    //    /|\ Occlusion
    // 1.0 |      ---------------\
    //     |      |             |  \
    //     |                         \
    //     |      |             |      \
    //     |                             \
    //     |      |             |          \
    //     |                                 \
    // ----|------|-------------|-------------|-------> zv
    //     0     Eps          zStart         zEnd
    
    float occlusion = 0.0f;
    if (distZ > g_SurfaceEpsilon)
    {
        float fadeLength = g_OcclusionFadeEnd - g_OcclusionFadeStart;
        // 当distZ由g_OcclusionFadeStart逐渐趋向于g_OcclusionFadeEnd，遮蔽值由1线性减小至0
        occlusion = saturate((g_OcclusionFadeEnd - distZ) / fadeLength);
    }
        return occlusion;
}

// Pass2 重新构建在观察空间(View Space)中的位置

void SSAO_VS(uint vertexID : SV_VertexID,
            out float4 posH : SV_Position,
            out float3 farPlanePoint : POSITION,
            out float2 texcoord : TEXCOORD)

{
    // 使用一个三角形覆盖NDC空间 
    // (-1, 1)________ (3, 1)
    //        |   |  /
    // (-1,-1)|___|/ (1, -1)   
    //        |  /
    // (-1,-3)|/  
    float2 grid = float2((vertexID << 1) & 2, vertexID & 2);
    float2 xy = grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    
    texcoord = grid * float2(1.0f, 1.0f);
    posH = float4(xy, 1.0f, 1.0f);
    farPlanePoint = g_FarPlanePoints[vertexID].xyz;
}

float4 SSAO_PS(float4 posH:SV_Position,
                float3 farPlanePoint : POSITION,
                float2 texcoord : TEXCOORD,
                uniform int sampleCount) : SV_Target
{
    // p -- 我们要计算的环境光遮蔽目标点
    // n -- 点p的法向量
    // q -- 点p处所在半球内的随机一点
    // r -- 有可能遮挡点p的一点
    
    float4 normalDepth = g_NormalDepthMap.SampleLevel(g_SamNormalDepth, texcoord, 0.0f);
    
    float3 n = normalDepth.xyz;
    float pz = normalDepth.w;
    
    float3 p = (pz / farPlanePoint.z) * farPlanePoint;
    
    // 
    // 获取随机向量并从[0, 1]^3映射到[-1, 1]^3
    float3 randVec = g_RandomVecMap.SampleLevel(g_SamRandomVec, 4.0f * texcoord, 0.0f).xyz;
    randVec = 2.0f * randVec - 1.0f;
    
    float occlusionSum = 0.0f;
    
    for (int i = 0; i < sampleCount; ++i)
    {
        // 随机偏移向量
        float3 offset = reflect(g_OffsetVectors[i].xyz, randVec);
        float flip = sign(dot(offset, n));
        
        float3 q = p + flip * g_OcclusionRadius * offset;
        
        // 将q进行投影，得到投影纹理坐标,用于后续对NormalDeptht贴图进行采样
        float4 projQ = mul(float4(q, 1.0f), g_ViewToTexSpace);
        projQ /= projQ.w;
        
        // 找到眼睛观察点q方向所能观察到的最近点r所处的深度值(有可能点r不存在，此时观察到
        // 的是远平面上一点)。为此，我们需要查看此点在深度图中的深度值
        float rz = g_NormalDepthMap.SampleLevel(g_SamNormalDepth, projQ.xy, 0.0f).w;
        
        float3 r = (rz / q.z) * q;
        
        // 测试点r是否遮蔽p
        //   - 点积dot(n, normalize(r - p))度量遮蔽点r到平面(p, n)前侧的距离。
        //   - 遮蔽权重的大小取决于遮蔽点与其目标点之间的距离。
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);
        float occlusion = dp * OcclusionFunction(distZ);
        
        occlusionSum += occlusion;
    }
    
    occlusionSum /= sampleCount;
   
    float access = 1.0f - occlusionSum;
    
    // 增强SSAO图的对比度，是的SSAO图的效果更加明显
    return saturate(pow(access, 4.0f));
}

// Pass3 模糊AO

float4 BilateralPS(float4 posH : SV_Position,
                    float2 texcoord : TEXCOORD) : SV_Target
{
    // 总是把中心值加进去计算
    float4 color = s_BlurWeights[g_BlurRadius] * g_InputImage.SampleLevel(g_SamBlur, texcoord, 0.0f);
    float totalWeight = s_BlurWeights[g_BlurRadius];
    
    float4 centerNormalDepth = g_NormalDepthMap.SampleLevel(g_SamBlur, texcoord, 0.0f);
    // 分拆出观察空间的法向量和深度
    float3 centerNormal = centerNormalDepth.xyz;
    float centerDepth = centerNormalDepth.w;
    
    for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        // 前面已经加上中心值了
        if (i == 0)
            continue;
#if defined BLUR_HORZ
        float2 offset = float2(g_TexelSize.x * i, 0.0f);
        float4 neighborNormalDepth = g_NormalDepthMap.SampleLevel(g_SamBlur, texcoord + offset, 0.0f);
#else
        float2 offset = float2(0.0f, g_TexelSize.y * i);
        float4 neighborNormalDepth = g_NormalDepthMap.SampleLevel(g_SamBlur, texcoord + offset, 0.0f);
#endif
        float3 neighborNormal = neighborNormalDepth.xyz;
        float neighborDepth = neighborNormalDepth.w;
        
        // 如果中心值和相邻值的深度或法向量相差太大，我们就认为当前采样点处于边缘区域，
        // 因此不考虑加入当前相邻值
        // 中心值直接加入
        
        if (dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f)
        {
            float weight = s_BlurWeights[i + g_BlurRadius];
            // 将相邻像素加入进行模糊
#if defined BLUR_HORZ
            color += weight * g_InputImage.SampleLevel(g_SamBlur, texcoord + offset, 0.0f);
#else
            color += weight * g_InputImage.SampleLevel(g_SamBlur, texcoord + offset, 0.0f);
#endif
            totalWeight += weight;
        }

    }
    // 通过让总权值变为1来补偿丢弃的采样像素
    return color / totalWeight;
}

float4 DebugAO_PS(float4 posH : SV_Position, 
                float2 texcoord : TEXCOORD) : SV_Target
{
    float depth = g_DiffuseMap.Sample(g_SamLinearWrap, texcoord).r;
    return float4(depth.rrr, 1.0f);
}

#endif
