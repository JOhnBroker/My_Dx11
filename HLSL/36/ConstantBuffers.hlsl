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
    float4  g_CascadeOffset[8];         // ShadowPT�����ƽ����
    float4  g_CascadeScale[8];          // ShadowPT�����������
    
    float   g_MinBorderPadding;
    float   g_MaxBorderPadding;
    float   g_MagicPower;
    int     g_VisualizeCascades;        // 1ʹ�ò�ͬ����ɫ���ӻ�������Ӱ��0���Ƴ���
    
    float   g_CascadeBlendArea;
    float   g_TexelSize;
    int     g_PCFBlurForLoopStart;      // ѭ����ʼֵ��5x5��PCF�˴�-2��ʼ
    int     g_PCFBlurForLoopEnd;        // ѭ������ֵ��5x5��PCF��Ӧ����Ϊ3
    
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