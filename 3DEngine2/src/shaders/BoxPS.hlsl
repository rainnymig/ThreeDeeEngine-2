#define MAX_LIGHTS 4

#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

struct LightSource
{
    float4 position;
    float4 direction;
    float4 color;
    float intensity;
    float linearAttenuation;
    float quadraticAttenuation;
    int type;
    bool isEnabled;
    int3 padding;
};

cbuffer Lights : register (b0)
{
    float4 eyePosition;
    LightSource lights[MAX_LIGHTS];
}

cbuffer Material : register (b1)
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specularColor;
    float4 emissiveColor;
    float ambientCoef;
    float diffuseCoef;
    float specularCoef;
    float emissiveCoef;
    int specularPower;
    bool useTexture;
    int2 padding;
}

struct PixelData
{
    float4 position         : SV_POSITION;
    float3 worldPosition    : POSWORLD;
    float3 normal           : NORMAL;
    float2 texCoord         : TEXCOORD;
};

struct LightResult
{
    float4 diffuse;
    float4 specular;
};

float4 computeLightDiffuse(float4 lightCol, float3 lightDir, float3 normal)
{
    return lightCol * max(dot(lightDir, normal), 0);
}

float4 computeLightSpecular(float4 lightCol, float3 lightDir, float3 viewDir, float3 normal, int specPow)
{
    //  blinn-phong
    float3 halfWayVector = normalize(lightDir + viewDir);
    return lightCol * max(pow(dot(normal, halfWayVector), specPow), 0);
}

LightResult computeDirectionalLighting(LightSource light, float3 viewDir, float3 normal)
{
    LightResult result;
    float3 lightDir = -normalize(light.direction.xyz);
    result.diffuse = computeLightDiffuse(light.color * light.intensity, lightDir, normal);
    result.specular = computeLightSpecular(light.color * light.intensity, lightDir, viewDir, normal, specularPower);
    //result.diffuse = computeLightDiffuse(light.color, lightDir, normal);
    //result.specular = computeLightSpecular(light.color, lightDir, viewDir, normal, specularPower);
    return result;
}

LightResult computePointLighting(LightSource light, float3 viewDir, float3 normal, float3 position)
{
    LightResult result;
    float3 lightDir = light.position.xyz - position;
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);
    float4 attenuatedLightColor = light.intensity * light.color * (1.0f / (1.0f + lightDistance * light.linearAttenuation +
        lightDistance * lightDistance * light.quadraticAttenuation));
    //float4 attenuatedLightColor = light.color * (1.0f / (1.0f + lightDistance * light.linearAttenuation +
    //    lightDistance * lightDistance * light.quadraticAttenuation));
    result.diffuse = computeLightDiffuse(attenuatedLightColor, lightDir, normal);
    result.specular = computeLightSpecular(attenuatedLightColor, lightDir, viewDir, normal, specularPower);
    return result;
}

float4 main(PixelData input) : SV_TARGET
{
    LightResult totalResult = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    LightResult partResult;

    float3 viewDir = normalize(eyePosition.xyz - input.worldPosition);

    [loop]
    for (int idx = 0; idx < MAX_LIGHTS; ++idx)
    {
        partResult.diffuse = float4(0, 0, 0, 0);
        partResult.specular = float4(0, 0, 0, 0);
        if (lights[idx].isEnabled)
        {
            switch (lights[idx].type)
            {
            case DIRECTIONAL_LIGHT:
                {
                    partResult = computeDirectionalLighting(lights[idx], viewDir, normalize(input.normal));
                }
                break;
            case POINT_LIGHT:
                {
                    partResult = computePointLighting(lights[idx], viewDir, normalize(input.normal), input.worldPosition);
                }
                break;
            case SPOT_LIGHT:
                {
                }
                break;
            }
            totalResult.diffuse += partResult.diffuse;
            totalResult.specular += partResult.specular;
        }
    }

    totalResult.diffuse = saturate(totalResult.diffuse);
    totalResult.specular = saturate(totalResult.specular);

    float4 emission = emissiveColor * emissiveCoef;
    float4 ambient = ambientColor * ambientCoef;
    float4 diffuse = diffuseColor * diffuseCoef * totalResult.diffuse;
    float4 specular = specularColor * specularCoef * totalResult.specular;

    return float4(saturate(emission + ambient + diffuse + specular).xyz, 1.0f);
}