// Include{s}
#include "GameObject.h"

#pragma region Constructor & Destructor
// Constructor
GameObject::GameObject(ID3D11Device* device, const char* OBJfilepath, const wchar_t* TEXfilepath, XMFLOAT3 Position,
	XMFLOAT3 Rotation, XMFLOAT3 Scale, int ID, std::string ObjectName)
{
	// Set member variables to the values passed in
	m_OBJfilepath = OBJfilepath;
	m_TEXfilepath = TEXfilepath;
	m_device = device;
	m_Position = Position;
	m_Rotation = Rotation;
	m_Scale = Scale;
	m_ID = ID;
	m_Name = ObjectName;

	// Set the world matrix to the transformation values
	XMStoreFloat4x4(&m_worldMatrix,
		XMMatrixIdentity() * XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z) * (
			XMMatrixRotationX(m_Rotation.x) * XMMatrixRotationY(m_Rotation.y) *
			XMMatrixRotationZ(m_Rotation.z)) * XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z));

	// OH YEAH IM MULTITHREADING BABY
	// Bullied to change the names of the threads (The voices)
	std::thread loadTexDataThread(&GameObject::LoadTexture, this);
	std::thread loadMeshDataThread(&GameObject::LoadMesh, this);

	loadTexDataThread.join();
	loadMeshDataThread.join();
}

// Destructor
GameObject::~GameObject()
{
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	if (m_meshData.VertexBuffer)
	{
		m_meshData.VertexBuffer->Release();
		m_meshData.VertexBuffer = nullptr;
	}

	if (m_meshData.IndexBuffer)
	{
		m_meshData.IndexBuffer->Release();
		m_meshData.IndexBuffer = nullptr;
	}
}
#pragma endregion

#pragma region Loading Methods
// Load texture for the game object
void GameObject::LoadTexture()
{
	if (std::wstring(m_TEXfilepath) == L"NULL")
	{
		return;
	}

	// Load the texture via the resource manager
	m_texture = ResourceManager::GetInstance()->LoadTexture(m_device, m_TEXfilepath);
}

// Load mesh for the game object
void GameObject::LoadMesh()
{
	if (std::string(m_OBJfilepath) == "NULL")
	{
		return;
	}

	// Load the mesh via the resource manager
	m_meshData = ResourceManager::GetInstance()->LoadMesh(m_device, m_OBJfilepath);
}
#pragma endregion

#pragma region Drawing Method
// Draw the game object
void GameObject::Draw(ConstantBuffer _cbData, ID3D11DeviceContext* _immediateContext, ID3D11Buffer* _constantBuffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};

	// Set the world matrix to current transformation values
	XMStoreFloat4x4(&m_worldMatrix,
		XMMatrixIdentity() * XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * (
			XMMatrixRotationX(m_Rotation.x) * XMMatrixRotationY(m_Rotation.y) *
			XMMatrixRotationZ(m_Rotation.z)) *
		XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z));

	_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix));
	_immediateContext->PSSetShaderResources(0, 1, &m_texture);
	_immediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &_cbData, sizeof(_cbData));
	_immediateContext->Unmap(_constantBuffer, 0);

	_immediateContext->IASetVertexBuffers(0, 1, &m_meshData.VertexBuffer, &m_meshData.VBStride,
		&m_meshData.VBOffset);
	_immediateContext->IASetIndexBuffer(m_meshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_immediateContext->DrawIndexed(m_meshData.IndexCount, 0, 0);
}
#pragma endregion

#pragma region Setters
// Set the position of the game object
void GameObject::SetPosition(float x, float y, float z)
{
	this->m_Position = XMFLOAT3(x, y, z);
}

// Set the rotation of the game object
void GameObject::SetRotation(float x, float y, float z)
{
	this->m_Rotation = XMFLOAT3(x, y, z);
}

// Set the scale of the game object
void GameObject::SetScale(float x, float y, float z)
{
	this->m_Scale = XMFLOAT3(x, y, z);
}
#pragma endregion

#pragma region Getters
// Get the rotation of the game object
XMFLOAT3 GameObject::GetRotation() const
{
	return m_Rotation;
}

// Get the position of the game object
XMFLOAT3 GameObject::GetPosition() const
{
	return m_Position;
}
#pragma endregion