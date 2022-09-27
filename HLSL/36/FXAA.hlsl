#ifndef FXAA_HLSL
#define FXAA_HLSL

#include "FullScreenTriangle.hlsl"

#ifndef FXAA_QUALITY__PRESET
#define FXAA_QUALITY__PRESET 39
#endif

//   FXAA 质量 - 低质量，中等抖动
#if (FXAA_QUALITY__PRESET == 10)
#define FXAA_QUALITY__PS 3 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.5, 3.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 11)
#define FXAA_QUALITY__PS 4 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 3.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 12)
#define FXAA_QUALITY__PS 5 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 13)
#define FXAA_QUALITY__PS 6 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 14)
#define FXAA_QUALITY__PS 7 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 15)
#define FXAA_QUALITY__PS 8 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 12.0 };
#endif

//   FXAA 质量 - 中等，较少抖动
#if (FXAA_QUALITY__PRESET == 20)
#define FXAA_QUALITY__PS 3 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.5, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 21)
#define FXAA_QUALITY__PS 4 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 22)
#define FXAA_QUALITY__PS 5 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 23)
#define FXAA_QUALITY__PS 6 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 24)
#define FXAA_QUALITY__PS 7 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 3.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 25)
#define FXAA_QUALITY__PS 8 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 26)
#define FXAA_QUALITY__PS 9
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 27)
#define FXAA_QUALITY__PS 10
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 28)
#define FXAA_QUALITY__PS 11
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 29)
#define FXAA_QUALITY__PS 12
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

//   FXAA 质量 - 高
#if (FXAA_QUALITY__PRESET == 39)
#define FXAA_QUALITY__PS 12
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

cbuffer CB : register(b0)
{
    float2 g_TexelSize;
    float g_QualitySubPix;
    float g_QualityEdgeThreshold;
    float g_QualityEdgeThresholdMin;
}

SamplerState g_SamplerLinearClamp : register(s0);
Texture2D<float4> g_TextureInput : register(t0);

// 亮度计算

float LinearRGBToLuminance(float3 LinearRGB)
{
    // return dot(LinearRGB, float3(0.299f, 0.587f, 0.114f));
    return dot(LinearRGB, float3(0.212671f, 0.715160f, 0.072169f));
    // return sqrt(dot(LinearRGB, float3(0.212671f, 0.715160, 0.072169)));
    // return log2(1 + dot(LinearRGB, float3(0.212671, 0.715160, 0.072169)) * 15) / 4;
}

float4 PS(float4 posH : SV_Position, float2 texCoord : TEXCOORD) : SV_Target
{
    float2 posM = texCoord;
    float4 color = g_TextureInput.SampleLevel(g_SamplerLinearClamp, texCoord, 0);
    
    //   N
    // W M E
    //   S
    float lumaM = LinearRGBToLuminance(color.rgb);
    float lumaS = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(0, 1)).rgb);
    float lumaE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, 0)).rgb);
    float lumaN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(0, -1)).rgb);
    float lumaW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, 0)).rgb);

    // 计算对比度，确定是否应用抗锯齿
    
    float lumaRangeMax = max(lumaM, max(max(lumaS, lumaN), max(lumaE, lumaW)));
    float lumaRangeMin = min(lumaM, min(min(lumaS, lumaN), min(lumaE, lumaW)));
    float lumaRange = lumaRangeMax - lumaRangeMin;
    bool earlyExit = lumaRange < max(g_QualityEdgeThresholdMin, lumaRange * g_QualityEdgeThreshold);
    
    if (earlyExit)
        return color;

    // 确定边界是局部水平的还是竖直的
    
    //  NW N NE          
    //  W  M  E
    //  WS S SE     
    //  edgeHorz = |(NW - W) - (W - WS)| + 2|(N - M) - (M - S)| + |(NE - E) - (E - SE)|
    //  edgeVert = |(NE - N) - (N - NW)| + 2|(E - M) - (M - W)| + |(SE - S) - (S - WS)|
    float lumaNW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, -1)).rgb);
    float lumaSE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, 1)).rgb);
    float lumaNE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, -1)).rgb);
    float lumaSW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, 1)).rgb);
    
    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    
    float edgeHorz = abs(lumaNWSW - 2.0 * lumaW) + abs(lumaNS - 2.0 * lumaM) * 2.0 + abs(lumaNESE - 2.0 * lumaE);
    float edgeVert = abs(lumaSWSE - 2.0 * lumaS) + abs(lumaWE - 2.0 * lumaM) * 2.0 + abs(lumaNWNE - 2.0 * lumaN);
    
    bool horzSpan = edgeHorz >= edgeVert;
    
    // 计算梯度、确定边界方向
    float luma1 = horzSpan ? lumaN : lumaW;
    float luma2 = horzSpan ? lumaS : lumaE;
    
    float gradient1 = luma1 - lumaM;
    float gradient2 = luma2 - lumaM;
    float gradientScaled = max(abs(gradient1), abs(gradient2)) * 0.25f;
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    
    // 当前像素沿梯度方向移动半个texel
    float lengthSign = horzSpan ? g_TexelSize.y : g_TexelSize.x;
    lengthSign = is1Steepest ? -lengthSign : lengthSign;
    
    float2 posB = posM.xy;
    // 半texel偏移
    if(!horzSpan)
        posB.x += lengthSign * 0.5;
    if(horzSpan)
        posB.y += lengthSign * 0.5;
    
    // 计算与posB相邻的两个像素的luma的平均值
    float luma3 = luma1 + lumaM;
    float luma4 = luma2 + lumaM;
    float lumaLocalAvg = luma3;
    if(!is1Steepest)
        lumaLocalAvg = luma4;
    lumaLocalAvg *= 0.5f;
    
    // 向两边进行遍历，直到遍历结束或达到非边缘点
    // 沿边界向两边偏移
    // 0    0    0
    // <-  posB ->
    // 1    1    1
    float2 offset;
    offset.x = (!horzSpan) ? 0.0f : g_TexelSize.x;
    offset.y = (horzSpan) ? 0.0f : g_TexelSize.y;
    float2 posN = posB - offset * s_SampleDistances[0];
    float2 posP = posB + offset * s_SampleDistances[0];
    
    float lumaEndN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posN, 0).rgb);
    float lumaEndP = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posP, 0).rgb);
    lumaEndN -= lumaLocalAvg;
    lumaEndP -= lumaLocalAvg;
    
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    bool doneNP = doneN && doneP;
    
    if(!doneN)
        posN -= offset * s_SampleDistances[1];
    if(!doneP)
        posP += offset * s_SampleDistances[1];
    
    if (!doneNP)
    {
        [unroll]
        for (int i = 2; i < FXAA_QUALITY__PS; ++i)
        {
            if(!doneN)
                lumaEndN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posN.xy, 0).rgb) - lumaLocalAvg;
            if (!doneP)
                lumaEndP = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posP.xy, 0).rgb) - lumaLocalAvg;
            
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            doneNP = doneN && doneP;
            
            if (!doneN)
                posN -= offset * s_SampleDistances[i];
            if (!doneP)
                posP += offset * s_SampleDistances[i];
            if(doneNP)
                break;
        }
    }
    
    // 分别计算到两个端点的距离
    float distN = horzSpan ? (posM.x - posN.x) : (posM.y - posN.y);
    float distP = horzSpan ? (posP.x - posM.x) : (posP.y - posM.y);

    // 看当前点到哪一个端点更近，取其距离
    bool directionN = distN < distP;
    float dist = min(distN, distP);
    
    // 两端点间的距离
    float spanLength = (distP + distN);
    
    // 朝着最近端点移动的像素偏移量
    float pixelOffset = -dist / spanLength + 0.5f;
    
    /*--------------------------------------------------------------------------*/
    
    // 当前像素的luma是否小于posB相邻的两个像素的luma的平均值
    bool isLumaMSmaller = lumaM < lumaLocalAvg;
    
    bool goodSpanN = (lumaEndN < 0.0) != isLumaMSmaller;
    bool goodSpanP = (lumaEndP< 0.0) != isLumaMSmaller;
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    
    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0f;
    
    /*--------------------------------------------------------------------------*/
    
    // 求3x3范围像素的亮度变化
    //      [1  2  1]
    // 1/12 [2 -12 2]
    //      [1  2  1]
    float subpixNSWE = lumaNS + lumaWE;
    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float subpixA = (2.0 * subpixNSWE + subpixNWSWNESE) * (1.0 / 12.0) - lumaM;
    
    // 基于这个亮度变化计算亚像素偏移量
    float subpixB = saturate(abs(subpixA) * (1.0f / lumaRange));
    float subpixC = (-2.0 * subpixB + 3.0) * subpixB * subpixB;
    float subpix = subpixC * subpixC * g_QualitySubPix;
    
    float pixelOffsetSubpix = max(pixelOffsetGood, subpix);
    /*--------------------------------------------------------------------------*/
    
    if(!horzSpan)
        posM.x += pixelOffsetSubpix * lengthSign;
    if(horzSpan)
        posM.y += pixelOffsetSubpix * lengthSign;

#ifdef DEBUG_OUTPUT
    return float4(1.0f - 2.0f * pixelOffsetSubpix, 2.0f * pixelOffsetSubpix, 0.0f, 1.0f);
#else
    return float4(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0).xyz, lumaM);
#endif
}

float4 PassThroughPS(float4 posH : SV_Position, float2 texCoord : TEXCOORD) : SV_Target
{
    uint2 pos = posH.xy;
    return g_TextureInput[pos];
}

#endif