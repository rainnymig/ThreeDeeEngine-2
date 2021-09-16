cbuffer TransformParameters : register(b0)
{
	matrix cameraWorldMatrix;
}

struct VertexData
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct PixelData
{
	float4 pos : SV_POSITION;
	float3 posWorld : POSWORLD;
	float2 texCoord : TEXCOORD;
};

PixelData main(VertexData input)
{
	PixelData output;
	output.pos = float4(sign(input.position.xy), 0.0f, 1.0f);
	output.posWorld = mul(cameraWorldMatrix, float4(input.position, 1.0f)).xyz;
	output.texCoord = input.texCoord;
	return output;
}
