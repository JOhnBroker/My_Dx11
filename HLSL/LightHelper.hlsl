#ifndef LIGHT_HELPER_HLSL
#define LIGHT_HELPER_HLSL

// �����
struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

// ���
struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 att;
    float pad;
};

// �۹��
struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 direction;
    float Spot;

    float3 att;
    float pad;
};

// ����������
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};



void ComputeDirectionalLight(Material mat, DirectionalLight L,
    float3 normal, float3 toEye,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �����������䷽���෴
    float3 lightVec = -L.direction;

    // ��ӻ�����
    ambient = mat.ambient * L.ambient;

    // ����������;����
    float diffuseFactor = dot(lightVec, normal);

    // չ�������⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
}


void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �ӱ��浽��Դ������
    float3 lightVec = L.position - pos;

    // ���浽���ߵľ���
    float d = length(lightVec);

    // �ƹⷶΧ����
    if (d > L.range)
        return;

    // ��׼��������
    lightVec /= d;

    // ���������
    ambient = mat.ambient * L.ambient;

    // ������;������
    float diffuseFactor = dot(lightVec, normal);

    // չ���Ա��⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // ���˥��
    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}


void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // // �ӱ��浽��Դ������
    float3 lightVec = L.position - pos;

    // ���浽��Դ�ľ���
    float d = length(lightVec);

    // ��Χ����
    if (d > L.range)
        return;

    // ��׼��������
    lightVec /= d;

    // ���㻷���ⲿ��
    ambient = mat.ambient * L.ambient;


    // �����������;��淴��ⲿ��
    float diffuseFactor = dot(lightVec, normal);

    // չ���Ա��⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // ���������Ӻ�˥��ϵ��
    float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.Spot);
    float att = spot / dot(L.att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}

float3 NormalSampleToWorldSpace(float3 normalMapSample,
    float3 unitNormalW,
    float4 tangentW)
{
    // ����ȡ���������е�ÿ��������[0, 1]��ԭ��[-1, 1]
    float3 normalT = 2.0f * normalMapSample - 1.0f;

    // ����λ����������ϵ�����߿ռ�
    float3 N = unitNormalW;
    float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N); // ʩ����������
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // ����͹�����������߿ռ�任����������ϵ
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
                       Texture2D shadowMap,
                       float4 shadowPosH,
                       float depthBias)
{
    // ͸�ӳ���
    shadowPosH.xyz /= shadowPosH.w;
    
    // NDC�ռ�����ֵ
    float depth = shadowPosH.z - depthBias;
    
    // ���������������µĿ��
    const float dx = SMAP_DX;
    
    float percentLit = 0.0f;
    
    // samShadowΪcompareValue <= sampleValueʱΪ1.0f(��֮Ϊ0.0f), �������ĸ����ؽ��в����Ƚ�
    // �����ݲ�����λ�ý���˫���Բ�ֵ
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(samShadow,
            shadowPosH.xy, depth, int2(i % 3 - 1, i / 3 - 1));
    }
    
    return percentLit /= 9.0f;
}

#endif