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
    int     g_VisualizeCascades;        // 1使用不同的颜色可视化级联阴影，0绘制场景
    int     g_PCFBlurForLoopStart;      // 循环初始值，5x5的PCF核从-2开始
    int     g_PCFBlurForLoopEnd;        // 循环结束值，5x5的PCF核应该设为3
    int     g_Pad;
    
    float   g_MinBorderPadding;
    float   g_MaxBorderPadding;
    float   g_ShadowBias;
    float   g_CascadeBlendArea;
    
    float   g_TexelSize;
    float3  g_LightDir;
    
    float4  g_CascadeFrustumsEyeSpaceDepthsFloat[2];
    float4  g_CascadeFrustumsEyeSpaceDepthsFloat4[8];
    
}

#endif