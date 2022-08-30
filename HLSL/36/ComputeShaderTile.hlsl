#ifndef COMPUTE_SHADER_TILE_HLSL
#define COMPUTE_SHADER_TILE_HLSL

#include "GBuffer.hlsl"
#include "FramebufferFlat.hlsl"
#include "ForwardTileInfo.hlsl"

RWStructuredBuffer<TileInfo> g_TilebufferRW : register(u0);

RWStructuredBuffer<uint2> g_Framebuffer : register(u1);

groupshared uint gs_MinZ;
groupshared uint gs_MaxZ;

groupshared uint gs_TileLightIndices[MAX_LIGHTS >> 3];
groupshared uint gs_TileNumLights;
groupshared uint tileLightIndices[MAX_LIGHT_INDICES];
groupshared uint gs_DepthMask;

groupshared uint gs_PerSamplePixels[COMPUTE_SHADER_TILE_GROUP_SIZE];
groupshared uint gs_NumPerSamplePixels;

void WriteSample(uint2 coords, uint sampleIndex, float4 value)
{
    g_Framebuffer[GetFramebufferSampleAddress(coords, sampleIndex)] = PackRGBA16(value);
}

uint PackCoords(uint2 coords)
{
    return coords.y << 16 | coords.x;
}

uint2 UnpackCoords(uint coords)
{
    return uint2((coords & 0xFFFF), coords >> 16);
}

void ConstructFrustumPlanes(uint3 groupId,float minTileZ,float maxTileZ,
                            out float4 frustumPlanes[6])
{
    // 注意：下面的计算每个分块都是统一的(例如：不需要每个线程都执行)，但代价低廉。
    // 我们可以只是先为每个分块预计算视锥平面，然后将结果放到一个常量缓冲区中...
    // 只有当投影矩阵改变的时候才需要变化，因为我们是在观察空间执行，
    // 然后我们就只需要计算近/远平面来贴紧我们实际的几何体。
    // 不管怎样，组同步/局部数据共享(Local Data Share, LDS)或全局内存寻找的开销可能和这小段数学一样多，但值得尝试。

    float2 tileScale = float2(g_FramebufferDimensions.xy) / COMPUTE_SHADER_TILE_GROUP_DIM;
    float2 tileBias = tileScale - 1.0f - 2.0f * float2(groupId.xy);
    
    // 计算当前分块视锥体的投影矩阵
    float4 c1 = float4(g_Proj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, g_Proj._22 * tileScale.y, -tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);
    
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, maxTileZ);
    
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
}

void ConstructLightDepthMask(PointLight light, float invDepthRange, out uint currDepthMask)
{
    uint minDepth = max(0.0f, min(31.0f, (light.posV.z - light.attenuationEnd) * invDepthRange));
    uint maxDepth = max(0.0f, min(31.0f, (light.posV.z + light.attenuationEnd) * invDepthRange));
    currDepthMask = 0xffffffff;
    currDepthMask >>= (31 - (maxDepth - minDepth));
    currDepthMask <<= minDepth;
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void ComputeShaderTileDeferredCS(uint3 groupId : SV_GroupID,
                                uint3 dispatchThreadId : SV_DispatchThreadID,
                                uint3 groupThreadId : SV_GroupThreadID,
                                uint groupIndex : SV_GroupIndex)
{
    // 获取表面数据，计算当前分块的视锥体
    uint2 globalCoords = dispatchThreadId.xy;
    
    SurfaceData surfaceSamples[MSAA_SAMPLES];
    ComputeSurfaceDataFromGBufferAllSamples(globalCoords, surfaceSamples);
    
    float minZSample = g_CameraNearFar.y;
    float maxZSample = g_CameraNearFar.x;
    {
        [unroll]
        for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
        {
            float viewSpaceZ = surfaceSamples[sample].posV.z;
            bool validPixel =
                viewSpaceZ >= g_CameraNearFar.x &&
                viewSpaceZ < g_CameraNearFar.y;
            [flatten]
            if (validPixel)
            {
                minZSample = min(minZSample, viewSpaceZ);
                maxZSample = max(maxZSample, viewSpaceZ);
            }
        }
    }
    
    if (groupIndex == 0)
    {
        gs_TileNumLights = 0;
        gs_NumPerSamplePixels = 0;
        gs_MinZ = 0x7F7FFFFF;
        gs_MaxZ = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    if (maxZSample >= minZSample)
    {
        InterlockedMin(gs_MinZ, asuint(minZSample));
        InterlockedMax(gs_MaxZ, asuint(maxZSample));
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float minTileZ = asfloat(gs_MinZ);
    float maxTileZ = asfloat(gs_MaxZ);
    float4 frustumPlanes[6];
    ConstructFrustumPlanes(groupId, minTileZ, maxTileZ, frustumPlanes);
    
    // 对当前分块(tile)进行光照裁剪
    uint totalLights, dummy;
    g_light.GetDimensions(totalLights, dummy);
    
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = g_light[lightIndex];
        
        //
        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustumPlanes[i], float4(light.posV, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(gs_TileNumLights, 1, listIndex);
            gs_TileLightIndices[listIndex] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    
    uint numLights = gs_TileNumLights;
    
    // 只处理在屏幕区域的像素(单个分块可能超出屏幕边缘)
    
    if (all(globalCoords < g_FramebufferDimensions.xy))
    {
        [branch]
        if (g_VisualizeLightCount)
        {
            [unroll]
            for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
            {
                WriteSample(globalCoords, sample, (float(gs_TileNumLights) / 255.0f).xxxx);
            }
        }
        else if (numLights > 0)
        {
            bool perSampleShading = RequiresPerSampleShading(surfaceSamples);
            // 逐样本着色可视化
            [branch]
            if (g_VisualizePerSampleShading && perSampleShading)
            {
                [unroll]
                for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
                {
                    WriteSample(globalCoords, sample, float4(1.0f, 0.0f, 0.0f, 1.0f));
                }
            }
            else
            {
                float3 lit = float3(0.0f, 0.0f, 0.0f);
                for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex)
                {
                    PointLight light = g_light[gs_TileLightIndices[tileLightIndex]];
                    AccumulateColor(surfaceSamples[0], light, lit);
                }

                WriteSample(globalCoords, 0, float4(lit, 1.0f));
                
                [branch]
                if (perSampleShading)
                {
#if DEFER_PER_SAMPLE
                    uint listIndex;
                    InterlockedAdd(gs_NumPerSamplePixels, 1, listIndex);
                    gs_PerSamplePixels[listIndex] = PackCoords(globalCoords);
#else               
                    // 对当前像素的其它样本进行着色
                    for (uint sample = 1; sample < MSAA_SAMPLES; ++sample)
                    {
                        float3 litSample = float3(0.0f, 0.0f, 0.0f);
                        for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex)
                        {
                            PointLight light = g_light[gs_TileLightIndices[tileLightIndex]];
                            AccumulateColor(surfaceSamples[sample], light, litSample);
                        }
                        WriteSample(globalCoords, sample, float4(litSample, 1.0f));
                    }
#endif
                }
                else
                {
                    [unroll]
                    for (uint sample = 1; sample < MSAA_SAMPLES; ++sample)
                    {
                        WriteSample(globalCoords, sample, float4(lit, 1.0f));
                    }
                }
            }
        }
        else
        {
            [unroll]
            for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
            {
                WriteSample(globalCoords, sample, float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
        }
    }
    
#if DEFER_PER_SAMPLE && MSAA_SAMPLES>1
    GroupMemoryBarrierWithGroupSync();
    // 现在处理那些需要逐样本着色的像素
    const uint shadingPassesPerPixel = MSAA_SAMPLES - 1;
    uint globalSamples = gs_NumPerSamplePixels * shadingPassesPerPixel;
    
    for (uint globalSample = groupIndex; globalSample < globalSamples; globalSample += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        uint listIndex = globalSample / shadingPassesPerPixel;
        uint sampleIndex = globalSample % shadingPassesPerPixel + 1;
        
        uint2 sampleCoords = UnpackCoords(gs_PerSamplePixels[listIndex]);
        SurfaceData surface = ComputeSurfaceDataFromGBufferSample(sampleCoords, sampleIndex);
        
        float3 lit = float3(0.0f, 0.0f, 0.0f);
        for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex)
        {
            PointLight light = g_light[gs_TileLightIndices[tileLightIndex]];
            AccumulateColor(surface, light, lit);
        }
        WriteSample(sampleCoords, sampleIndex, float4(lit, 1.0f));
    }
#endif
    
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM,COMPUTE_SHADER_TILE_GROUP_DIM,1)]
void ComputeShaderTileForwardCS(uint3 groupId : SV_GroupID,
                                uint3 dispatchThreadId : SV_DispatchThreadID,
                                uint3 groupThreadId : SV_GroupThreadID,
                                uint groupIndex : SV_GroupIndex)
{
    // 获取深度数据，计算当前分块的视锥体
    uint2 globalCoords = dispatchThreadId.xy;
    
    float minZSample = g_CameraNearFar.y;
    float maxZSample = g_CameraNearFar.x;
    {
        [unroll]
        for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
        {
            float zBuffer = g_GBufferTextures[3].Load(globalCoords, sample);
            float viewSpaceZ = g_Proj._m32 / (zBuffer - g_Proj._m22);
            
            bool validPixel =
                viewSpaceZ >= g_CameraNearFar.x &&
                viewSpaceZ < g_CameraNearFar.y;
            [flatten]
            if (validPixel)
            {
                minZSample = min(minZSample, viewSpaceZ);
                maxZSample = max(maxZSample, viewSpaceZ);
            }
        }
    }
    if (groupIndex == 0)
    {
        gs_TileNumLights = 0;
        gs_NumPerSamplePixels = 0;
        gs_MinZ = 0x7F7FFFFF;
        gs_MaxZ = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();
    if (maxZSample >= minZSample)
    {
        InterlockedMin(gs_MinZ, asuint(minZSample));
        InterlockedMax(gs_MaxZ, asuint(maxZSample));
    }
    GroupMemoryBarrierWithGroupSync();
    
    float minTileZ = asfloat(gs_MinZ);
    float maxTileZ = asfloat(gs_MaxZ);
    float4 frustumPlanes[6];
    ConstructFrustumPlanes(groupId, minTileZ, maxTileZ, frustumPlanes);
    
    // 对当前分块(tile)进行光照裁剪
    uint totalLights, dummy;
    g_light.GetDimensions(totalLights, dummy);
    
    uint2 dispatchWidth = (g_FramebufferDimensions.x + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint tilebufferIndex = groupId.y * dispatchWidth + groupId.x;
    
    [loop]
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = g_light[lightIndex];
        
        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustumPlanes[i], float4(light.posV, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(gs_TileNumLights, 1, listIndex);
            g_TilebufferRW[tilebufferIndex].tileLightIndices[listIndex] = lightIndex;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    if (groupIndex == 0)
    {
        g_TilebufferRW[tilebufferIndex].tileNumLights = gs_TileNumLights;
    }
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void PointLightCullingForwardCS(uint3 groupId : SV_GroupID,
                                uint3 dispatchThreadId : SV_DispatchThreadID,
                                uint3 groupThreadId : SV_GroupThreadID,
                                uint groupIndex : SV_GroupIndex)
{
    // 获取深度数据，计算当前分块的视锥体
    uint2 globalCoords = dispatchThreadId.xy;
    float minZSample = g_CameraNearFar.y;
    float maxZSample = g_CameraNearFar.x;
    float msaaDepthMask[MSAA_SAMPLES];
    uint sample = 0;
    
    [unroll]
    for (sample = 0; sample < MSAA_SAMPLES; ++sample)
    {
        float zBuffer = g_GBufferTextures[3].Load(globalCoords, sample);
        float viewSpaceZ = g_Proj._m32 / (zBuffer - g_Proj._m22);
        
        bool validPixel =
            viewSpaceZ >= g_CameraNearFar.x &&
            viewSpaceZ < g_CameraNearFar.y;
        [flatten]
        if (validPixel)
        {
            msaaDepthMask[sample] = viewSpaceZ;
            minZSample = min(minZSample, viewSpaceZ);
            maxZSample = max(maxZSample, viewSpaceZ);
        }
    }

    if (groupIndex == 0)
    {
        gs_TileNumLights = 0;
        gs_NumPerSamplePixels = 0;
        gs_MinZ = 0x7F7FFFFF;
        gs_MaxZ = 0;
        gs_DepthMask = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();
    if (maxZSample >= minZSample)
    {
        InterlockedMin(gs_MinZ, asuint(minZSample));
        InterlockedMax(gs_MaxZ, asuint(maxZSample));
    }
    GroupMemoryBarrierWithGroupSync();
    
    float minTileZ = asfloat(gs_MinZ);
    float maxTileZ = asfloat(gs_MaxZ);
    float4 frustumPlanes[6];
    ConstructFrustumPlanes(groupId, minTileZ, maxTileZ, frustumPlanes);
    
    // 2.5D Culling
    // 将minTileZ 到maxTileZ 之间划分为32部分[0-31]，当存在当前深度时，则将对应为设置为1 
    
    const float invDepthRange = 31.0f / (gs_MaxZ - gs_MinZ);
    uint currDepthMask = 0;
    [unroll]
    for (sample = 0; sample < MSAA_SAMPLES; ++sample)
    {
        uint bitPlace = max(0.0f, min(31.0f, msaaDepthMask[sample] * invDepthRange));
        currDepthMask |= 1 << bitPlace;
    }
    InterlockedOr(gs_DepthMask, currDepthMask);
    
    GroupMemoryBarrierWithGroupSync();
    
    // 对当前分块(tile)进行光照裁剪
    uint totalLights, dummy;
    g_light.GetDimensions(totalLights, dummy);
    
    uint2 dispatchWidth = (g_FramebufferDimensions.x + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint tilebufferIndex = groupId.y * dispatchWidth + groupId.x;

    // 每条线程处理 totalLights / 线程组的数量
    [loop]
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = g_light[lightIndex];
        uint lightDepthMask = 0;
        bool inFrustum = true;
        ConstructLightDepthMask(light, invDepthRange, lightDepthMask);
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustumPlanes[i], float4(light.posV, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }
        [branch]
        if (inFrustum &&(currDepthMask & lightDepthMask))
        {
            uint listIndex;
            InterlockedAdd(gs_TileNumLights, 1, listIndex);
            g_TilebufferRW[tilebufferIndex].tileLightIndices[listIndex] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    if (groupIndex == 0)
    {
        g_TilebufferRW[tilebufferIndex].tileNumLights = gs_TileNumLights;
    }
}

#endif