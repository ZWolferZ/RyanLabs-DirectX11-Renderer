// Load the skybox and sampler
TextureCube skybox : register(t0);
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
	float3 texCoord : TEXCOORD;
};

// Vertex shader
VS_Out VS_main(float3 position : POSITION)
{
	VS_Out output;

	// Transform position to world space
	float4 worldPos = mul(float4(position, 1.0f), World);
	float4 viewPos = mul(worldPos, View);
	float4 projPos = mul(viewPos, Projection);


	output.position = projPos.xyww;
	output.texCoord = position;

	return output;
}

// Pixel shader
float4 PS_main(VS_Out input) : SV_TARGET
{
	// Pixelate filter
	if (pixelateFilter == 1)
	{
		input.texCoord = floor(input.texCoord * pixelationAmount) / pixelationAmount;
	}

	// Sample the skybox texture
	float4 texColor = skybox.Sample(bilinearSampler, input.texCoord);

	return texColor;
}
