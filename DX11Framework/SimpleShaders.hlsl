// Load the texture and sampler
Texture2D diffuseTex : register(t0);
SamplerState bilinearSampler : register(s0);

// ALWAYS MAKE SURE TO HAVE THE SAME STRUCTURE IN THE C++ CODE
// ALWAYS MAKE SURE IS PACKED
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
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 PosW : POSITION0;
	float3 NormalW : NORMALW;
	float2 texCoord : TEXCOORD;
};

// Vertex shader
VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 Texcoord : TEXCOORD)
{
	VS_Out output = (VS_Out)0;
	float4 Pos4 = float4(Position, 1.0f);

	if (waveFilter == 1)
	{
		// Wave Effect
		float wave = sin(Position.x * 1.0f + count * 0.5f) * 0.2f;
		wave += sin(Position.z * 1.0f + count * 0.5f * 1.3f) * 0.2f;
		Pos4.y += wave;
	}
	if (waveFilterX == 1)
	{
		// Wave X Effect
		float wave = sin(Position.x * 1.0f + count * 0.5f) * 0.2f;
		wave += sin(Position.z * 1.0f + count * 0.5f * 1.3f) * 0.22f;
		Pos4.x += wave;
	}


	// Transform normal to world space
	float4 NormW = float4(Normal, 0.0f);
	output.NormalW = mul(NormW, World);
	output.NormalW = normalize(output.NormalW.xyz);
	output.normal = normalize(Normal);
	output.PosW = mul(Pos4, World).xyz;

	// Transform position to world space
	float4 viewPos = mul(Pos4, World);
	viewPos = mul(viewPos, View);
	output.position = mul(viewPos, Projection);

	// Pass texture coordinates
	output.texCoord = Texcoord.xy;

	return output;
}

// Pixel shader
float4 PS_main(VS_Out input, float3 PosW : POSITION0, float3 NormalW : NORMALW) : SV_TARGET
{
	// Pixelate Filter
	float2 pixelatedTexCoord = input.texCoord;

	if (pixelateFilter == 1)
	{
		pixelatedTexCoord = floor(input.texCoord * pixelationAmount) / pixelationAmount;
	}

	// Sample the texture
	float4 texColor = diffuseTex.Sample(bilinearSampler, pixelatedTexCoord);

	// Calculate Diffuse lighting 
	float4 DiffuseAmount = saturate(dot(-LightDir, normalize(input.NormalW)));
	float4 DiffuseOut = DiffuseAmount * (DiffuseMaterial * DiffuseLight);
	float4 DiffuseOutTex = DiffuseAmount * (texColor * DiffuseLight);

	// Calculate gooch shading
	float4 GoochOut = DiffuseAmount * float4(1.0f, 0.0f, 0.0f, 1.0f) + (1 - DiffuseAmount) * float4(
		0.0f, 0.0f, 1.0f, 1.0f);

	// Calculate Ambient lighting
	float4 AmbientOut = AmbientLight * AmbientMaterial;
	float4 AmbientOutTex = AmbientLight * texColor;

	// Calculate Specular lighting
	float3 viewDir = normalize(CameraPosition - normalize(input.PosW));
	float3 reflectDir = reflect(LightDir, normalize(input.NormalW));
	float SpecularIntensity = saturate(dot(reflectDir, viewDir));
	float SpecularAmount = pow(SpecularIntensity, SpecPower);
	float SpecularOut = ((SpecularLight * SpecPower) * SpecularMaterial) * SpecularAmount;

	// Final color
	float4 finalColor;

	// Final state checks
	if (LightON == 1)
	{
		if (hasTexture == 1)
		{
			finalColor = DiffuseOutTex + AmbientOutTex + SpecularOut;
		}
		else
		{
			finalColor = DiffuseOut + AmbientOut + SpecularOut;
		}
	}
	else if (hasTexture == 1)
	{
		finalColor = texColor;
	}
	else
	{
		finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	if (goochShading == 1)
	{
		finalColor = GoochOut;
	}


	return finalColor;
}
