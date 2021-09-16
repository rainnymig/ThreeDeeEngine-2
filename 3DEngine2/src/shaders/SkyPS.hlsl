#define MAX_LIGHTS 4

texture2D renderTargetTexture : register(t0);
texture2D depthStencilTexture : register(t1);

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

cbuffer constParams : register(b1)
{
    float4 heavenColor;
    float4 hellColor;
	float zNear;
	float zFar;
    float2 padding1;
}

sampler linearSampler: register(s0);

struct PixelData
{
    float4 pos : SV_POSITION;
    float3 posWorld : POSWORLD;
    float2 texCoord : TEXCOORD;
};

//float recoverLinearDepth(float nonLinearDepth)
//{
//    return -(zNear * zFar) / ((zFar - zNear) * nonLinearDepth - zFar);
//}

float4 main(PixelData input) : SV_TARGET
{
    float depth = depthStencilTexture.SampleLevel(linearSampler, input.texCoord, 0).r;
    if (depth < 1.0f)
    {
        return renderTargetTexture.SampleLevel(linearSampler, input.texCoord, 0);
    }
    else
    {
        float3 direction = normalize(input.posWorld - eyePosition.xyz);
        float t = (direction.y + 1.0f) / 2.0f;
        float4 color = lerp(hellColor, heavenColor, sin(t));
        return color;
    }
}
