#ifndef CASCADED_SHADOW_HLSL
#define CASCADED_SHADOW_HLSL

#include "ConstantBuffers.hlsl"

// 使用偏导，将shadow map中的texels映射到正在渲染的图元的观察空间平面上
// 该深度将会用于比较并减少阴影走样
// 这项技术是开销昂贵的，且假定对象是平面较多的时候才有效
#ifndef USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG
#define USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG 0
#endif

// 允许在不同级联之间对阴影值混合。当shadow maps比较小
// 且artifacts在两个级联之间可见的时候最为有效
#ifndef BLEND_BETWEEN_CASCADE_LAYERS_FLAG
#define BLEND_BETWEEN_CASCADE_LAYERS_FLAG 0
#endif

// 有两种方法为当前像素片元选择合适的级联：
// Interval-based Selection 将视锥体的深度分区与像素片元的深度进行比较
// Map-based Selection 找到纹理坐标在shadow map范围中的最小级联
#ifndef SELECT_CASCADE_BY_INTERVAL_FLAG
#define SELECT_CASCADE_BY_INTERVAL_FLAG 0
#endif 

// 级联数目
#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif

Texture2DArray g_ShadowMap : register(t10);
SamplerComparisonState g_SamShadow : register(s10);

static const float4 s_CascadeColorMultiplier[8] =
{
    float4(1.5f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 1.5f, 0.0f, 1.0f),
    float4(0.0f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 1.5f, 0.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f, 1.0f),
    float4(0.0f, 1.0f, 5.5f, 1.0f),
    float4(0.5f, 3.5f, 0.75f, 1.0f)
};

//--------------------------------------------------------------------------------------
// 为阴影空间的texels计算对应光照空间
//--------------------------------------------------------------------------------------
void CalculateRightAndUpTexelDepthDeltas(float3 shadowTexDDX, float3 shadowTexDDY,
                                        out float upTexDepthWeight,
                                        out float rightTexDepthWeight)
{
    float2x2 matScreenToShadow = float2x2(shadowTexDDX.xy, shadowTexDDY.xy);
    float det = determinant(matScreenToShadow);
    float invDet = 1.0f / det;
    float2x2 matShadowToScreen = float2x2(
        matScreenToShadow._22 * invDet, matScreenToShadow._12 * -invDet,
        matScreenToShadow._21 * -invDet, matScreenToShadow._11 * invDet);
    float2 rightShadowTexelLocation = float2(g_TexelSize, 0.0f);
    float2 upShadowTexelLocation = float2(0.0f, g_TexelSize);
    
    float2 rightTexelDepthRatio = mul(rightShadowTexelLocation, matShadowToScreen);
    float2 upTexelDepthRatio = mul(upShadowTexelLocation, matShadowToScreen);

    upTexDepthWeight =
        upTexelDepthRatio.x * shadowTexDDX.z +
        upTexelDepthRatio.y * shadowTexDDY.z;
    rightTexDepthWeight =
        rightTexelDepthRatio.x * shadowTexDDX.z +
        rightTexelDepthRatio.y * shadowTexDDY.z;
}

//--------------------------------------------------------------------------------------
// 使用PCF采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float CalculatePCFPercentLit(int currentCascadeIndex, 
                            float4 shadowTexCoord, 
                            float rightTexelDepthDelta, 
                            float upTexelDepthDelta, 
                            float blurSize)
{
    float percentLit = 0.0f;
    for (int x = g_PCFBlurForLoopStart; x < g_PCFBlurForLoopEnd; ++x)
    {
        for (int y = g_PCFBlurForLoopStart; y < g_PCFBlurForLoopEnd; ++y)
        {
            float depthCmp = shadowTexCoord.z;
            // 一个非常简单的解决PCF深度偏移问题的方案是使用一个偏移值
            // 不幸的是，过大的偏移会导致Peter-panning（阴影跑出物体）
            // 过小的偏移又会导致阴影失真
            depthCmp -= g_ShadowBias;
            if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
            {
                depthCmp += rightTexelDepthDelta * (float) x + upTexelDepthDelta * (float) y;
            }
            percentLit += g_ShadowMap.SampleCmpLevelZero(g_SamShadow,
                float3(
                    shadowTexCoord.x + (float) x * g_TexelSize,
                    shadowTexCoord.y + (float) y * g_TexelSize,
                    (float) currentCascadeIndex
            ),
            depthCmp);

        }
    }
    percentLit /= blurSize;
    return percentLit;
}

//--------------------------------------------------------------------------------------
// 计算两个级联之间的混合量 及 混合将会发生的区域
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval(int currentCascadeIndex,
                                    inout float pixelDepth, 
                                    inout float currentPixelsBlendBandLocation, 
                                    out float blendBetweenCascadesAmount)
{
    // blendBandLocation = 1 - depth/F[0] or
    // blendBandLocation = 1 - (depth-F[0]) / (F[i]-F[0])
    // blendBandLocation位于[0, g_CascadeBlendArea]时，进行[0, 1]的过渡
    float blendInterval = g_CascadeFrustumsEyeSpaceDepthsFloat4[currentCascadeIndex].x;
    
    if (currentCascadeIndex > 0)
    {
        int blendIntervalbelowIndex = currentCascadeIndex - 1;
        pixelDepth -= g_CascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
        blendInterval -= g_CascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
    }
    
    currentPixelsBlendBandLocation = 1.0f - pixelDepth / blendInterval;
    blendBetweenCascadesAmount = currentPixelsBlendBandLocation / g_CascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// 计算两个级联之间的混合量 及 混合将会发生的区域
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForMap(float4 shadowMapTexCoord, 
                                inout float currentPixelsBlendBandLocation, 
                                inout float blendBetweenCascadesAmount)
{
    // blendBandLocation = min(tx, ty, 1-tx, 1-ty);
    // blendBandLocation位于[0, g_CascadeBlendArea]时，进行[0, 1]的过渡
    float2 distanceToOne = float2(1.0f - shadowMapTexCoord.x, 1.0f - shadowMapTexCoord.y);
    currentPixelsBlendBandLocation = min(shadowMapTexCoord.x, shadowMapTexCoord.y);
    float currentPixelsBlendBandLocation2 = min(distanceToOne.x, distanceToOne.y);
    currentPixelsBlendBandLocation = min(currentPixelsBlendBandLocation, currentPixelsBlendBandLocation2);
    
    blendBetweenCascadesAmount = currentPixelsBlendBandLocation / g_CascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// 计算级联显示色或者对应的过渡色
//--------------------------------------------------------------------------------------
float4 GetCascadeColorMultipler(int currentCascadeIndex,
                                int nextCascadeIndex,
                                float blendBetweenCascadesAmount)
{
    return lerp(s_CascadeColorMultiplier[currentCascadeIndex],
                s_CascadeColorMultiplier[nextCascadeIndex], 
                blendBetweenCascadesAmount);
}

//--------------------------------------------------------------------------------------
// 计算级联阴影
//--------------------------------------------------------------------------------------
float CalculateCascadedShadow(float4 shadowMapTexCoordViewSpace, 
                            float currentPixelDepth, 
                            out int currentCascadeIndex,
                            out int nextCascadeIndex, 
                            out float blendBetweenCascadesAmount)
{
    float4 shadowMapTexCoord = 0.0f;
    float4 shadowMapTexCoord_blend = 0.0f;
    float4 visualizeCascadeColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float percentLit = 0.0f;
    float percentLit_blend = 0.0f;
    
    float upTextDepthWeight = 0.0f;
    float rightTextDepthWeight = 0.0f;
    float upTextDepthWeight_blend = 0.0f;
    float rightTextDepthWeight_blend = 0.0f;
    
    float blurSize = g_PCFBlurForLoopEnd - g_PCFBlurForLoopStart;
    blurSize *= blurSize;
    
    int cascadeFound = 0;
    nextCascadeIndex = 1;
    
    //
    // 确定级联，变换阴影纹理坐标
    //
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        currentCascadeIndex = 0;
        //                               Depth
        // /-+-------/----------------/----+-------/----------/
        // 0 N     F[0]     ...      F[i]        F[i+1] ...   F
        // Depth > F[i] to F[0] => index = i+1
        if (CASCADE_COUNT_FLAG > 1)
        {
            float4 currentPixelDepthVec = currentPixelDepth;
            float4 cmpVec1 = (currentPixelDepthVec > g_CascadeFrustumsEyeSpaceDepthsFloat[0]);
            float4 cmpVec2 = (currentPixelDepthVec > g_CascadeFrustumsEyeSpaceDepthsFloat[1]);
            float index = dot(float4(CASCADE_COUNT_FLAG > 0,
                                    CASCADE_COUNT_FLAG > 1,
                                    CASCADE_COUNT_FLAG > 2,
                                    CASCADE_COUNT_FLAG > 3),
                            cmpVec1) + 
                        dot(float4(CASCADE_COUNT_FLAG > 4,
                                    CASCADE_COUNT_FLAG > 5,
                                    CASCADE_COUNT_FLAG > 6,
                                    CASCADE_COUNT_FLAG > 7),
                            cmpVec2);
            index = min(index, CASCADE_COUNT_FLAG - 1);
            currentCascadeIndex = index;
        }
        shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[currentCascadeIndex] + g_CascadeOffset[currentCascadeIndex];
    }
    
    // Map-Based Selection
    if (!SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        currentCascadeIndex = 0;
        if (CASCADE_COUNT_FLAG == 1)
        {
            shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[0] + g_CascadeOffset[0];
        }
        if (CASCADE_COUNT_FLAG > 1)
        {
            for (int cascadeIndex = 0; cascadeIndex < CASCADE_COUNT_FLAG && cascadeFound == 0; ++cascadeIndex)
            {
                shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[cascadeIndex] + g_CascadeOffset[cascadeIndex];
                if (min(shadowMapTexCoord.x, shadowMapTexCoord.y) > g_MinBorderPadding &&
                    max(shadowMapTexCoord.x, shadowMapTexCoord.y) < g_MaxBorderPadding)
                {
                    currentCascadeIndex = cascadeIndex;
                    cascadeFound = 1;
                }
            }
        }
    }
    
    // 计算当前级联的PCF
    float3 shadowMapTexCoordDDX;
    float3 shadowMapTexCoordDDY;
    if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
    {
        shadowMapTexCoordDDX = ddx(shadowMapTexCoordViewSpace);
        shadowMapTexCoordDDY = ddy(shadowMapTexCoordViewSpace);
        shadowMapTexCoordDDX *= g_CascadeScale[currentCascadeIndex];
        shadowMapTexCoordDDY *= g_CascadeScale[currentCascadeIndex];
        CalculateRightAndUpTexelDepthDeltas(shadowMapTexCoordDDX, shadowMapTexCoordDDY,
                                            upTextDepthWeight, rightTextDepthWeight);
    }
    visualizeCascadeColor = s_CascadeColorMultiplier[currentCascadeIndex];
    percentLit = CalculatePCFPercentLit(currentCascadeIndex, shadowMapTexCoord,
                                rightTextDepthWeight, upTextDepthWeight, blurSize);
    
    // 在两个级联之间进行混合
    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
    {
        nextCascadeIndex = min(CASCADE_COUNT_FLAG - 1, currentCascadeIndex + 1);
    }
    blendBetweenCascadesAmount = 1.0f;
    float currentPixelsBlendBandLocation = 1.0f;
    
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
        {
            CalculateBlendAmountForInterval(currentCascadeIndex, currentPixelDepth,
                currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
        }
    }
    else
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
        {
            CalculateBlendAmountForMap(shadowMapTexCoord, 
                currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
        }
    }
    
    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
    {
        if (currentPixelsBlendBandLocation < g_CascadeBlendArea)
        {
            // 计算下一级联的投影纹理坐标
            shadowMapTexCoord_blend = shadowMapTexCoordViewSpace * g_CascadeScale[nextCascadeIndex] + g_CascadeOffset[nextCascadeIndex];
            if (currentPixelsBlendBandLocation < g_CascadeBlendArea)
            {
                if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
                {
                    CalculateRightAndUpTexelDepthDeltas(shadowMapTexCoordDDX, shadowMapTexCoordDDY,
                                                        upTextDepthWeight_blend, rightTextDepthWeight_blend);
                }
                percentLit_blend = CalculatePCFPercentLit(nextCascadeIndex, shadowMapTexCoord,
                                                        rightTextDepthWeight_blend, upTextDepthWeight_blend, blurSize);
            
                percentLit = lerp(percentLit_blend, percentLit, blendBetweenCascadesAmount);
            }
        }
    }
    return percentLit;
}



#endif