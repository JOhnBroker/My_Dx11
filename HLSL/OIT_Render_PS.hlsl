#include "OIT.hlsli"

#define MAX_SORTED_PIXELS 8

static FragmentData g_SortedPixels[MAX_SORTED_PIXELS];

void SortPixelInPlace(int numPixels)
{
    FragmentData temp;
    for (int i = 1; i < numPixels; ++i)
    {
        for (int j = i - 1; j >= 0; --j)
        {
            if (g_SortedPixels[j].depth < g_SortedPixels[j + 1].depth)
            {
                temp = g_SortedPixels[j];
                g_SortedPixels[j] = g_SortedPixels[j + 1];
                g_SortedPixels[j + 1] = temp;
            }
            else
            {
                break;
            }
        }
    }
}

float4 PS(float4 posH:SV_Position) : SV_TARGET
{
    // 取出当前像素位置对应的背景色
    float4 currColor = g_BackGround.Load(int3(posH.xy, 0));
    
    // 取出当前像素位置链表长度
    uint2 vPos = (uint2) posH.xy;
    int startOffsetAddress = 4 * (g_FrameWidth * vPos.y + vPos.x);
    int numPixels = 0;
    uint offset = g_StartOffsetBuffer.Load(startOffsetAddress);
    
    FLStaticNode element;
    
    // 取出链表所有节点
    while (offset != 0xFFFFFFFF)
    {
        element = g_FLBuffer[offset];
        g_SortedPixels[numPixels++] = element.data;
        offset = (numPixels >= MAX_SORTED_PIXELS) ? 0xFFFFFFFF : element.next;
    }
    
    // 对所有取出的像素片元按深度值从大到小排序
    SortPixelInPlace(numPixels);
    
    // 使用SrcAlpha-InvSrcAlpha混合
    for (int i = 0; i < numPixels; ++i)
    {
        float4 pixelColor = UnpackColorFromUInt(g_SortedPixels[i].color);
        currColor.xyz = lerp(currColor.xyz, pixelColor.xyz, pixelColor.w);
    }
    
    // 返回手工混合的颜色
        return currColor;
}