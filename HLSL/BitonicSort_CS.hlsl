#include "BitonicSort.hlsli"
#define BITONIC_BLOCK_SIZE 512

groupshared uint shared_data[BITONIC_BLOCK_SIZE];

[numthreads(BITONIC_BLOCK_SIZE, 1, 1)]
void CS(uint3 Gid : SV_GroupID,
    uint3 DTid: SV_DispatchThreadID,
    uint3 GTid: SV_GroupThreadID,
    uint GI:SV_GroupIndex)
{
    //д�빲������
    shared_data[GI] = g_Data[DTid.x];
    GroupMemoryBarrierWithGroupSync();
    
    // ����
    [unroll]
    for (uint j = g_level >> 1; j > 0; j >>= 1)
    {
        uint smallerIndex = GI & ~j;
        uint largerIndex = GI | j;
        bool isSmallerIndex = (GI == smallerIndex);
        // �ж��ǵ���/�ݼ�
        bool isDescending = (bool) (g_DescendMask & DTid.x);
        uint res = ((shared_data[smallerIndex] <= shared_data[largerIndex]) == (isDescending == isSmallerIndex)) ?
            shared_data[largerIndex] : shared_data[smallerIndex];
        GroupMemoryBarrierWithGroupSync();
        
        shared_data[GI] = res;
        GroupMemoryBarrierWithGroupSync();
    }
    // ������
    g_Data[DTid.x] = shared_data[GI];
}