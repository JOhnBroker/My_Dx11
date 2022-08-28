#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "Rendering.hlsl"

#ifndef MSAA_SAMPLES
#define MSAA_SAMPLES 1
#endif

// GBuffer����س��ù��ߺͽṹ
struct GBuffer
{
    float4 normal_specular : SV_Target0;
    float4 albedo : SV_Target1;
    float2 posZGrad : SV_Target2; // ( d(x+1,y)-d(x,y), d(x,y+1)-d(x,y) )
};

// ����GBuffer������Ȼ�����(���һ��Ԫ��)  t1-t4 
Texture2DMS<float4, MSAA_SAMPLES> g_GBufferTextures[4] : register(t1);

// ���߱���
float2 EncodeSphereMap(float3 normal)
{
    return normalize(normal.xy) * (sqrt(-normal.z * 0.5f + 0.5f));
}

// ���߽���
float3 DecodeSphereMap(float2 encoded)
{
    float4 nn = float4(encoded, 1, -1);
    float l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2 + float3(0, 0, -1);
}

float3 ComputePositionViewFromZ(float2 posNdc, float viewSpaceZ)
{
    float2 screenSpaceRay = float2(posNdc.x / g_Proj._m00,
                                   posNdc.y / g_Proj._m11);
    float3 posV;
    posV.z = viewSpaceZ;
    posV.xy = screenSpaceRay.xy * posV.z;
    return posV;    
}

SurfaceData ComputeSurfaceDataFromGBufferSample(uint2 posViewport, uint sampleIndex)
{
    GBuffer rawData;
    rawData.normal_specular = g_GBufferTextures[0].Load(posViewport.xy, sampleIndex).xyzw;
    rawData.albedo = g_GBufferTextures[1].Load(posViewport.xy, sampleIndex).xyzw;
    rawData.posZGrad = g_GBufferTextures[2].Load(posViewport.xy, sampleIndex).xy;
    float zBuffer = g_GBufferTextures[3].Load(posViewport.xy, sampleIndex).x;
 
    float2 gbufferDim;
    uint dummy;
    g_GBufferTextures[0].GetDimensions(gbufferDim.x, gbufferDim.y, dummy);

    // ������Ļ/�ü��ռ���������ڵ�λ��
    // ע�⣺��Ҫ����DX11���ӿڱ任����������λ��(x+0.5, y+0.5)λ��
    // ע�⣺��ƫ��ʵ���Ͽ�����CPUԤ���㵫�����ŵ�������������ȡʵ���ϱ����������¼������һЩ
    float2 screenPixelOffset = float2(2.0f, -2.0f) / gbufferDim;
    float2 posNdc = (float2(posViewport.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
    float2 posNdcX = posNdc + float2(screenPixelOffset.x, 0.0f);
    float2 posNdcY = posNdc + float2(0.0f, screenPixelOffset.y);
    
    SurfaceData data;
    
    float viewSpaceZ = g_Proj._m32 / (zBuffer - g_Proj._m22);
    
    data.posV = ComputePositionViewFromZ(posNdc, viewSpaceZ);
    data.posV_DX = ComputePositionViewFromZ(posNdcX, viewSpaceZ + rawData.posZGrad.x) - data.posV;
    data.posV_DY = ComputePositionViewFromZ(posNdcY, viewSpaceZ + rawData.posZGrad.y) - data.posV;
    
    data.normalV = DecodeSphereMap(rawData.normal_specular.xy);
    data.albedo = rawData.albedo;
    
    data.specularAmount = rawData.normal_specular.z;
    data.specularPower = rawData.normal_specular.w;
    
    return data;
}

void ComputeSurfaceDataFromGBufferAllSamples(uint2 posViewport,
                                            out SurfaceData surface[MSAA_SAMPLES])
{
    [unroll]
    for (uint i = 0; i < MSAA_SAMPLES; ++i)
    {
        surface[i] = ComputeSurfaceDataFromGBufferSample(posViewport, i);
    }
}

bool RequiresPerSampleShading(SurfaceData surface[MSAA_SAMPLES])
{
    bool perSample = false;
    
    const float maxZDelta = abs(surface[0].posV_DX.z) + abs(surface[0].posV_DY.z);
    const float minNormalDot = 0.99f; // �����Լ8�ȵķ��߽ǶȲ���
    
    [unroll]
    for (uint i = 1; i < MSAA_SAMPLES; ++i)
    {
        perSample = perSample || 
            abs(surface[i].posV.z - surface[0].posV.z) > maxZDelta;
        perSample = perSample ||
            dot(surface[i].normalV, surface[0].normalV) < minNormalDot;
    }
    
    return perSample;
}

// ʹ�������(1)/������(0)��־����ʼ��ģ������ֵ
void RequiresPerSampleShadingPS(float4 posViewPort : SV_Position)
{
    SurfaceData surfaceSamples[MSAA_SAMPLES];
    ComputeSurfaceDataFromGBufferAllSamples(uint2(posViewPort.xy), surfaceSamples);
    bool perSample = RequiresPerSampleShading(surfaceSamples);
    
    [flatten]
    if (!perSample)
    {
        discard;
    }
}

//--------------------------------------------------------------------------------------
// G-buffer ��Ⱦ
//--------------------------------------------------------------------------------------
void GBufferPS(VertexPosHVNormalVTex input, out GBuffer outputGBuffer)
{
    SurfaceData surface = ComputeSurfaceDataFromGeometry(input);
    outputGBuffer.normal_specular = float4(EncodeSphereMap(surface.normalV),
                                            surface.specularAmount,
                                            surface.specularPower);
    outputGBuffer.albedo = surface.albedo;
    outputGBuffer.posZGrad = float2(ddx_coarse(surface.posV.z),
                                    ddy_coarse(surface.posV.z));
}

// �������ɷ�����ͼ
float4 DebugNormalPS(float4 posViewport : SV_Position) : SV_Target
{
    float4 normal_specular = g_GBufferTextures[0].Load(posViewport.xy, 0).xyzw;
    float3 normalV = DecodeSphereMap(normal_specular.xy);
    float3 normalW = mul(float4(normalV, 0.0f), g_InvView).xyz;
    
    // ������ȾĿ��ΪsRGB������ȥ��٤��У�����۲�ԭʼ������ͼ
    // [-1, 1] => [0, 1]
    return pow(float4((normalW + 1.0f) / 2.0f, 1.0f), 2.2f);
}

// �������ֵ�ݶ���ͼ
float4 DebugPosZGradPS(float4 posViewport : SV_Position) : SV_Target
{
    float2 posZGrad = g_GBufferTextures[2].Load(posViewport.xy, 0).xy;
  
    return pow(float4(posZGrad, 0.0f, 1.0f), 2.2f);
}

#endif