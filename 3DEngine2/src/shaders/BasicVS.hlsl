cbuffer TransformMatrices : register(b0)
{
    matrix worldMatrix;
    matrix inversedWorldMatrix;
    matrix viewProjMatrix;
}

struct VertexData
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOutputData
{
    float4 position         : SV_POSITION;
    float3 worldPosition    : POSWORLD;
    float3 normal           : NORMAL;
    float2 texCoord         : TEXCOORD;
};

VertexOutputData main(VertexData input)
{
    VertexOutputData output;
    matrix worldViewProjMatrix = mul(viewProjMatrix, worldMatrix);
    float4 v = float4(input.position, 1.0f);
    output.position = mul(worldViewProjMatrix, v);
    output.worldPosition = mul(worldMatrix, v).xyz;
    output.normal = normalize(mul(inversedWorldMatrix, float4(input.normal, 1.0f)).xyz);
    output.texCoord = input.texCoord;

    return output;
}