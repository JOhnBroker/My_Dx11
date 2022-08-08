#include "Blur.hlsli"

groupshared float4 g_Cache[CacheSize];

[numthreads(1, N, 1)]
void main(int3 GTid : SV_GroupThreadID,
    int3 DTid : SV_DispatchThreadID)
{
    // ͨ����д�����̴߳洢�������ٴ���ĸ��ء���Ҫ��N�����ؽ���ģ����������ģ���뾶��
    // ������Ҫ����N + 2 * BlurRadius������
    
    // ���߳���������N���̡߳�Ϊ�˻�ȡ�����2*BlurRadius�����أ�����Ҫ��2*BlurRadius��
    // �̶߳���ɼ�һ����������
    
    if (GTid.y < g_BlurRadius)
    {
        int y = max(DTid.y - g_BlurRadius, 0);
        g_Cache[GTid.y] = g_Input[int2(DTid.x, y)];
    }

    uint texWidth, texHeight;
    g_Input.GetDimensions(texWidth, texHeight);
    
    if (GTid.y >= N - g_BlurRadius)
    {
        int y = min(DTid.y + g_BlurRadius, texHeight - 1);
        g_Cache[GTid.y + 2 * g_BlurRadius] = g_Input[int2(DTid.x, y)];
    }
    
    // ������д��Cache�Ķ�Ӧλ��
    // ���ͼ�α߽紦��Խ������������ǯλ����
    g_Cache[GTid.y + g_BlurRadius] = g_Input[min(DTid.xy, float2(texWidth, texHeight) - 1)];
    
    // �ȴ������߳��������
    GroupMemoryBarrierWithGroupSync();
    
    // ��ʼ��ÿ�����ؽ��л��
    float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        int k = GTid.y + g_BlurRadius + i;
        blurColor += s_Weights[i + g_BlurRadius] * g_Cache[k];
    }
    
    g_Output[DTid.xy] = blurColor;

}