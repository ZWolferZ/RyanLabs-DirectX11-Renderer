#pragma once

// Include{s}
#include <codecvt>
#include <d3d11_1.h>
#include <directxmath.h>
#include "DDSTextureLoader.h"
#include <SpriteBatch.h>  // Using DirectX11 Toolkit by Microsoft Library (Not Mine!)
#include <SpriteFont.h> // Using DirectX11 Toolkit by Microsoft Library (Not Mine!)
#include <memory>
#include <wrl.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <map>           // For fast searching when re-creating the index buffer
#include <nlohmann/json.hpp> // Using nlohmann.json Library (Not Mine!)
#include <fstream>
#include <iostream>
#include <thread>

// Enforce the use of the DirectX namespace, on most of the files
using namespace DirectX;

#pragma region Structs

// Struct to hold constant buffer data
struct ConstantBuffer
{
	XMMATRIX Projection;
	XMMATRIX View;
	XMMATRIX World;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 DiffuseMaterial;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;
	XMFLOAT3 LightDir;
	float count;
	XMFLOAT4 SpecularLight;
	XMFLOAT4 SpecularMaterial;
	XMFLOAT3 CameraPosition;
	float SpecPower;
	uint32_t hasTexture;
	uint32_t waveFilter;
	uint32_t LightON;
	uint32_t waveFilterX;
	uint32_t pixelateFilter;
	uint32_t goochShading;
	float pixelationAmount;
	XMFLOAT3 padding;
};

// Struct to hold mesh data
struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

// Struct to hold simple vertex data
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp(this, &other, sizeof(SimpleVertex)) > 0;
	};
};

// Struct to hold key states
enum KeyState
{
	Key_UP,
	Key_DOWN
};

#pragma endregion
