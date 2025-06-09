#pragma once
// Broken and Unfinished Terrain Class (Bonus Points for Trying?)
// Include{s}
#include "Structures.h"

class Terrain
{
public:
#pragma region Constructor & Destructor
	// Constructor calls other methods to build the terrain
	Terrain(ID3D11Device* device);

	~Terrain();
#pragma endregion

#pragma region Public Methods
	// Loads the heightmap from a raw file
	void LoadHeightmap(int heightmapWidth, int heightmapHeight, std::string heightmapFileName);

	// Builds the heightmaps
	void BuildHeightMaps(ID3D11Device* device);

	// Builds the vertex buffer for a flat grid
	void BuildFlatGridVB(ID3D11Device* device);

	// Builds the index buffer for a flat grid
	void BuildFlatGridIB(ID3D11Device* device);
#pragma endregion

#pragma region Public Member Variables
	ID3D11Buffer* m_TriFlatGridVB;
	ID3D11Buffer* m_TriFlatGridIB;
	std::vector<unsigned int> m_indices;
	XMFLOAT4X4 m_matrix;
	ID3D11ShaderResourceView* m_HeightMapSRV;
#pragma endregion

private:
#pragma region Private Member Variables
	int m_cellSpacing = 1;
	int m_NumPatchVertCols = 400;
	int m_NumPatchVertRows = 400;
	std::vector<float> m_heightMapData;
	int m_HeightmapWidth;
	int m_HeightmapHeight;
	std::string m_HeightmapFileName;
#pragma endregion
};
