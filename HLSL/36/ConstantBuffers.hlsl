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
    int     g_VisualizeCascades;        // 1ʹ�ò�ͬ����ɫ���ӻ�������Ӱ��0���Ƴ���
    int     g_PCFBlurForLoopStart;      // ѭ����ʼֵ��5x5��PCF�˴�-2��ʼ
    int     g_PCFBlurForLoopEnd;        // ѭ������ֵ��5x5��PCF��Ӧ����Ϊ3
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