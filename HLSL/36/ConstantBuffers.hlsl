#ifndef CONSTANT_BUFFERS_HLSL
#define CONSTANT_BUFFERS_HLSL

cbuffer CBChangesEveryInstanceDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    matrix g_WorldView;
    matrix g_WorldViewProj;
}

cbuffer CBCascadedShadow : register(b1)
{
    matrix  g_ShadowView;
    float4  g_CascadeOffset[8];         // ShadowPT矩阵的平移量
    float4  g_CascadeScale[8];          // ShadowPT矩阵的缩放量
    
    float   g_MinBorderPadding;
    float   g_MaxBorderPadding;
    float   g_MagicPower;
    int     g_VisualizeCascades;        // 1使用不同的颜色可视化级联阴影，0绘制场景
    
    float   g_CascadeBlendArea;
    float   g_TexelSize;
    int     g_PCFBlurForLoopStart;      // 循环初始值，5x5的PCF核从-2开始
    int     g_PCFBlurForLoopEnd;        // 循环结束值，5x5的PCF核应该设为3
    
    float   g_PCFDepthBias;
    float3  g_LightDir;
    
    float   g_LightBleedingReduction;
    float   g_EvsmPosExp;
    float   g_EVsmNegExp;
    int     g_16BitShadow;
    
    float4  g_CascadeFrustumsEyeSpaceDepthsFloat[2];
    float4  g_CascadeFrustumsEyeSpaceDepthsFloat4[8] = (float[8]) g_CascadeFrustumsEyeSpaceDepthsData;
    
}

#endif