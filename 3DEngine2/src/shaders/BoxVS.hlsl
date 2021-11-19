cbuffer BoxData: register(b0)
{
	matrix worldMatrix;
	matrix viewProjectionMatrix;
	matrix inversedTransposedWorldMatrix;
	//float4 boxWorldCenterAndScale;
};

struct VertexData
{
	float3 centerPosition		: POSITION;
	uint vertex					: TEXCOORD;	//	it's not real tex coord though
};

struct PixelData
{
	float4 position				: SV_POSITION;
	float3 worldPosition		: POSWORLD;
	float3 normal				: NORMAL;
	float2 texCoord				: TEXCOORD;
};

const static float3 vertexPositions[8] = {
	{ -0.5f, -0.5f, -0.5f },
	{ -0.5f, -0.5f,  0.5f },
	{ -0.5f,  0.5f, -0.5f },
	{ -0.5f,  0.5f,  0.5f },
	{  0.5f, -0.5f, -0.5f },
	{  0.5f, -0.5f,  0.5f },
	{  0.5f,  0.5f, -0.5f },
	{  0.5f,  0.5f,  0.5f }
};

const static float3 normals[6] = {
	{ -1.0f,  0.0f,  0.0f },
	{  0.0f, -1.0f,  0.0f },
	{  0.0f,  0.0f, -1.0f },
	{  1.0f,  0.0f,  0.0f },
	{  0.0f,  1.0f,  0.0f },
	{  0.0f,  0.0f,  1.0f },
};

const static float2 texCoords[4] = {
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 0.0f },
	{ 1.0f, 1.0f },
};

PixelData main(VertexData input)
{
	PixelData output;
	
	matrix worldViewProjection = mul(viewProjectionMatrix, worldMatrix);

	uint vertexPosIdx = input.vertex & 0x7;
	uint vertexNormalIdx = (input.vertex & 0x38) >> 3;
	uint vertexTexIdx = (input.vertex & 0xC0) >> 6;

	//float3 boxWorldCenter = boxWorldCenterAndScale.xyz;
	//float boxScale = boxWorldCenterAndScale.w;

	//float4 vertexPos = float4(boxWorldCenter + input.centerPosition + boxScale * vertexPositions[vertexPosIdx], 1.0f);
	float4 vertexPos = float4(input.centerPosition + vertexPositions[vertexPosIdx], 1.0f);
	float3 normal = normals[vertexNormalIdx];
	float2 texCoord = texCoords[vertexTexIdx];

	output.position = mul(worldViewProjection, vertexPos);
	output.worldPosition = mul(worldMatrix, vertexPos).xyz;
	output.normal = normalize(mul(inversedTransposedWorldMatrix, float4(normal, 0.0f)).xyz);
	output.texCoord = texCoord;

	return output;
}
