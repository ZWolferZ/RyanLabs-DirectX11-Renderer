#pragma once
#include "ResourceManager.h"

class GameObject
{
public:
#pragma region Constructors
	// Constructor
	GameObject(ID3D11Device* device, const char* OBJfilepath, const wchar_t* TEXfilepath, XMFLOAT3 Position,
		XMFLOAT3 Rotation, XMFLOAT3 Scale, int ID, std::string ObjectName);

	// Destructor
	~GameObject();
#pragma endregion

#pragma region Load Methods
	// Loads the texture for the game object
	void LoadTexture();

	// Loads the mesh for the game object
	void LoadMesh();
#pragma endregion

#pragma region Draw Method
	// Draws the game object
	void Draw(ConstantBuffer cbData, ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer);
#pragma endregion

#pragma region Setters
	// Sets the scale of the game object
	void SetScale(float x, float y, float z);

	// Sets the world matrix of the game object
	void SetWorldMatrix(XMMATRIX worldMatrix) { XMStoreFloat4x4(&m_worldMatrix, worldMatrix); }

	// Sets the shader resource of the game object
	void SetShaderResource(ID3D11ShaderResourceView* texture) { m_texture = texture; }

	// Sets the mesh data of the game object
	void SetMeshData(MeshData meshData) { m_meshData = meshData; }

	// Sets the position of the game object
	virtual void SetPosition(float x, float y, float z);

	// Sets the rotation of the game object
	virtual void SetRotation(float x, float y, float z);
#pragma endregion

#pragma region Getters
	// Gets the rotation of the game object
	// Returns XMFLOAT3 - The gameobject rotation as a float3
	XMFLOAT3 GetRotation() const;

	// Gets the position of the game object
	// Returns XMFLOAT3 - The gameobject position as a float3
	XMFLOAT3 GetPosition() const;

	// Gets the shader resource of the game object
	// Returns ID3D11ShaderResourceView - The gameobject texture
	ID3D11ShaderResourceView** GetShaderResource() { return &m_texture; }

	// Gets the mesh data of the game object
	// Returns MeshData - The gameobject MeshData
	MeshData* GetMeshData() { return &m_meshData; }

#pragma endregion

#pragma region Public Member Variables
	// Position of the game object
	XMFLOAT3 m_Position = { 0, 0, 0 };

	// Rotation of the game object
	XMFLOAT3 m_Rotation = { 0, 0, 0 };

	// Scale of the game object
	XMFLOAT3 m_Scale = { 1, 1, 1 };

	// ID of the game object
	int m_ID = 0;

	// Name of the game object
	std::string m_Name = "NULL";
#pragma endregion

private:
#pragma region Private Member Variables
	// Texture of the game object
	ID3D11ShaderResourceView* m_texture = nullptr;

	// Mesh data of the game object
	MeshData m_meshData = {};

	// World matrix of the game object
	XMFLOAT4X4 m_worldMatrix = {};

	// File path to the OBJ file
	const char* m_OBJfilepath = nullptr;

	// File path to the texture file
	const wchar_t* m_TEXfilepath = nullptr;

	// Device for rendering
	ID3D11Device* m_device = nullptr;
#pragma endregion
};
