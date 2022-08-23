#ifndef RENDERING_HLSL
#define RENDERING_HLSL

#include "FullScreenTriangle.hlsl"
#include "ConstantBuffers.hlsl"

// ���ߺ���
float linstep(float min_value, float max_value, float value)
{
    return saturate((value - min_value) / (max_value - min_value));
}

// ���ν׶�

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexPosHVNormalVTex
{
    float4 posH : SV_Position;
    float3 posV : POSITION;
    float3 normalV : NORMAL;
    float2 texCoord : TEXCOORD;
};

VertexPosHVNormalVTex GeometryVS(VertexPosNormalTex input)
{
    VertexPosHVNormalVTex output;
    
    output.posH = mul(float4(input.posL, 1.0f), g_WorldViewProj);
    output.posV = mul(float4(input.posL, 1.0f), g_WorldView).xyz;
    output.normalV = mul(float4(input.normalL, 0.0f), g_WorldInvTransposeView).xyz;
    output.texCoord = input.texCoord;
    
    return output;
}

float3 ComputeFaceNormal(float3 pos)
{
    return cross(ddx_coarse(pos), ddy_coarse(pos));
}

struct SurfaceData
{
    float3 posV;
    float3 posV_DX;
    float3 posV_DY;
    float3 normalV;
    float4 albedo;
    float specularAmount;
    float specularPower;
};

SurfaceData ComputeSurfaceDataFromGeometry(VertexPosHVNormalVTex input)
{
    SurfaceData surface;
    surface.posV = input.posV;
    
    // ��/�����������뵱ǰ���ص�λ�ò�
    surface.posV_DX = ddx_coarse(surface.posV);
    surface.posV_DY = ddy_coarse(surface.posV);
    
    // �ñ��淨�߿���������ṩ�ķ���
    float3 faceNormal = ComputeFaceNormal(surface.posV);
    surface.normalV = normalize(g_FaceNormals ? faceNormal : input.normalV);
    
    surface.albedo = g_DiffuseMap.Sample(g_Sam, input.texCoord);
    surface.albedo.rgb = g_LightingOnly ? float3(1.0f, 1.0f, 1.0f) : surface.albedo.rgb;
    
    surface.specularAmount = 0.9f;
    surface.specularPower = 25.0f;
    
    return surface;
}

// ���ս׶�

struct PointLight
{
    float3 posV;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};

StructuredBuffer<PointLight> g_light : register(t5);

// ����ֳ�diffuse/specular����ӳٹ���ʹ��
void AccumulatePhong(float3 normal, float3 lightDir, float3 viewDir, float3 lightContrib, float specularPower,
                    inout float3 litDiffuse, inout float3 litSpecular)
{
    float NdotL = dot(normal, lightDir);
    [flatten]
    if (NdotL > 0.0f)
    {
        float3 r = reflect(lightDir, normal);
        float RdotV = max(0.0f, dot(r, viewDir));
        float specular = pow(RdotV, specularPower);
        
        litDiffuse += lightContrib * NdotL;
        litSpecular += lightContrib * specular;
    }
}

void AccumulateDiffuseSpecular(SurfaceData surface, PointLight light,
                            inout float3 litDiffuse, inout float3 litSpecular)
{
    float3 dirToLight = light.posV - surface.posV;
    float distToLight = length(dirToLight);
    
    [branch]
    if (distToLight < light.attenuationEnd)
    {
        float attenuation = linstep(light.attenuationEnd, light.attenuationBegin, distToLight);
        dirToLight *= rcp(distToLight);
        AccumulatePhong(surface.normalV, dirToLight, normalize(surface.posV),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
    }
}

void AccumulateColor(SurfaceData surface,PointLight light,
                inout float3 litColor)
{
    float3 dirToLight = light.posV - surface.posV;
    float distToLight = length(dirToLight);
    
    [branch]
    if (distToLight < light.attenuationEnd)
    {
        float attenuation = linstep(light.attenuationEnd, light.attenuationBegin, distToLight);
        dirToLight *= rcp(distToLight);
        
        float3 litDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 litSpecular = float3(0.0f, 0.0f, 0.0f);
        AccumulatePhong(surface.normalV, dirToLight, normalize(surface.posV),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
        litColor += surface.albedo.rgb * (litDiffuse + surface.specularAmount * litSpecular);
    }
}

#endif