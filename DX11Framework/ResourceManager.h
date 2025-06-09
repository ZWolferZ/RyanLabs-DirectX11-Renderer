#pragma once

// Include{s}
#include "Structures.h"
#include "OBJLoader.h"

class ResourceManager
{
public:
#pragma region Singleton

	// Only allow one instance of the ResourceManager
	// Singleton Pattern
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	// Get the instance of the ResourceManager
	// Returns ResourceManager* - The instance of the ResourceManager
	static ResourceManager* GetInstance()
	{
		static ResourceManager instance;
		return &instance;
	}
#pragma endregion

#pragma region LoadMethods
	// Checks to see if the texture has already been loaded, if not, loads it
	ID3D11ShaderResourceView* LoadTexture(ID3D11Device* _device, const wchar_t* path);

	// Checks to see if the mesh has already been loaded, if not, loads it
	MeshData LoadMesh(ID3D11Device* _device, const std::string& path);
#pragma endregion

private:
#pragma region Constructor & Destructor
	// Default constructor
	ResourceManager() = default;

	// Destructor
	~ResourceManager();
#pragma endregion

#pragma region Member Variables
	// Vectors to store texture paths and their corresponding resources
	std::vector<std::wstring> m_texturePaths;
	std::vector<ID3D11ShaderResourceView*> m_Textures;

	// Vectors to store mesh paths and their corresponding data
	std::vector<std::string> m_MeshPaths;
	std::vector<MeshData> m_Meshes;
#pragma endregion
};
