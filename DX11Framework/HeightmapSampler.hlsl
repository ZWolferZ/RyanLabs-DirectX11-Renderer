// Load the heightmap texture and sampler
Texture2D heightmapTexture : register(t0);
SamplerState bilinearSampler : register(s0);

// Constant buffer passed from the application
cbuffer

ConstantBuffer: 

register (b0)
{
	float4x4 Projection;
	float4x4 View;
	float4x4 World;
	float4 DiffuseLight;
	float4 DiffuseMaterial;
	float4 AmbientLight;
	float4 AmbientMaterial;
	float3 LightDir;
	float count;
	float4 SpecularLight;
	float4 SpecularMaterial;
	float3 CameraPosition;
	float SpecPower;
	uint hasTexture;
	uint waveFilter;
	uint LightON;
	uint waveFilterX;
	uint pixelateFilter;
	uint goochShading;
	float pixelationAmount;
	float3 padding;
}

// Input structure
struct VS_Out
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

// Vertex shader
VS_Out VS_main(float3 position : POSITION, float2 Texcoord : TEXCOORD)
{
	VS_Out output = (VS_Out)0;
	float4 Pos4 = float4(position, 1.0f);

	// Sample the heightmap texture
	float height = heightmapTexture.SampleLevel(bilinearSampler, Texcoord, 0).r;
	Pos4.y = height;

	// Transform position to world space
	float4 worldPos = mul(Pos4, World);
	float4 viewPos = mul(worldPos, View);
	output.position = mul(viewPos, Projection);

	// Pass texture coordinates
	output.texCoord = Texcoord;

	return output;
}

// Pixel shader
float4 PS_main(VS_Out input) : SV_TARGET
{
	// Sample the heightmap texture
	float4 texColor = heightmapTexture.Sample(bilinearSampler, input.texCoord);
	return texColor;
}
