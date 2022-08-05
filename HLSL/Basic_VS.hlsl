#include "Basic.hlsli"

VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    
    // ����ˮ��ʱ
    if (g_WavesEnabled)
    {
        // ʹ��ӳ�䵽[0,1]x[0,1]���������������в���
        vIn.posL.y += g_DisplacementMap.SampleLevel(g_SamLinearWrap, vIn.tex, 0.0).r;
        // ʹ�����޲�ַ����㷨����
        float left = g_DisplacementMap.SampleLevel(g_SamPointClamp, vIn.tex, 0.0f, uint2(-1, 0)).r;
        float right = g_DisplacementMap.SampleLevel(g_SamPointClamp, vIn.tex, 0.0f, uint2(1, 0)).r;
        float top = g_DisplacementMap.SampleLevel(g_SamPointClamp, vIn.tex, 0.0f, uint2(0, -1)).r;
        float bottom = g_DisplacementMap.SampleLevel(g_SamPointClamp, vIn.tex, 0.0f, uint2(0, 1)).r;
        vIn.normalL = normalize(float3(-right + left, 2.0f * g_GridSpatialStep, bottom - top));
    }
    
    vector posW = mul(float4(vIn.posL, 1.0f), g_World);
    
    vOut.posW = posW.xyz;
    vOut.posH = mul(posW, g_ViewProj);
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.tex = mul(float4(vIn.tex, 0.0f, 1.0f), g_TexTransform).xy;
    return vOut;
}