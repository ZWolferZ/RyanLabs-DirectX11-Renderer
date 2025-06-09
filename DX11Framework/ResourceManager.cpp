#include "ResourceManager.h"

#pragma region Load Methods
ID3D11ShaderResourceView* ResourceManager::LoadTexture(ID3D11Device* _device, const wchar_t* path)
{
	// Iterate through the texture paths, if the path is found, return the texture
	for (int i = 0; i < m_texturePaths.size(); ++i)
	{
		if (m_texturePaths[i] == path)
		{
			return m_Textures[i];
		}
	}

	// Load the texture from the path, if no texture is found
	ID3D11ShaderResourceView* texture;
	HRESULT hr = CreateDDSTextureFromFile(_device, path, nullptr, &texture);
	if (FAILED(hr)) { return nullptr; }

	m_texturePaths.push_back(path);
	m_Textures.push_back(texture);

	return texture;
}

MeshData ResourceManager::LoadMesh(ID3D11Device* _device, const std::string& path)
{
	// Iterate through the mesh paths, if the path is found, return the mesh
	for (int i = 0; i < m_MeshPaths.size(); ++i)
	{
		if (m_MeshPaths[i] == path)
		{
			return m_Meshes[i];
		}
	}

	// Load the mesh from the path, if no mesh is found
	MeshData meshData = OBJLoader::Load(path.c_str(), _device);
	m_MeshPaths.push_back(path);
	m_Meshes.push_back(meshData);

	return meshData;
}
#pragma endregion

#pragma region Destructor
// Destructor
ResourceManager::~ResourceManager()
{
	m_Textures.clear();
	m_Meshes.clear();
	m_texturePaths.clear();
	m_MeshPaths.clear();
}
#pragma endregion