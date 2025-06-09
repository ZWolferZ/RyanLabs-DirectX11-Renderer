// Broken and Unfinished Terrain Class (Bonus Points for Trying?)

// Include{s}
#include "Terrain.h"

#pragma region Constructor & Destructor

// Build the terrain, from the heightmap
Terrain::Terrain(ID3D11Device* device)
{
	LoadHeightmap(513, 513, "Textures\\Heightmap_513x513.raw");
	BuildHeightMaps(device);

	std::thread buildVBThread(&Terrain::BuildFlatGridVB, this, device);
	std::thread buildIBThread(&Terrain::BuildFlatGridIB, this, device);
	buildVBThread.join();
	buildIBThread.join();

	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
}

// Destructor
Terrain::~Terrain()
{
	if (m_TriFlatGridVB)
	{
		m_TriFlatGridVB->Release();
		m_TriFlatGridVB = nullptr;
	}

	if (m_TriFlatGridIB)
	{
		m_TriFlatGridIB->Release();
		m_TriFlatGridIB = nullptr;
	}

	m_indices.clear();
	m_heightMapData.clear();
}

#pragma endregion

#pragma region Heightmap Methods
void Terrain::LoadHeightmap(int heightmapWidth, int heightmapHeight, std::string heightmapFileName)
{
	m_HeightmapWidth = heightmapWidth;
	m_HeightmapHeight = heightmapHeight;
	m_HeightmapFileName = heightmapFileName;

	std::vector<unsigned char> in(m_HeightmapWidth * m_HeightmapHeight);

	// Read in the raw file
	std::ifstream inFile;
	inFile.open(m_HeightmapFileName.c_str(), std::ios_base::binary);
	if (inFile)
	{
		inFile.read((char*)&in[0], static_cast<std::streamsize>(in.size()));
		inFile.close();
	}

	m_heightMapData.resize(m_HeightmapWidth * m_HeightmapHeight);

	float heightScale = 10.0f;
	for (unsigned int i = 0; i < m_HeightmapHeight * m_HeightmapWidth; i++)
	{
		// Normalise the heightmap data and add it to the vector
		m_heightMapData[i] = (in[i] / 255.0f) * heightScale;
	}
}

void Terrain::BuildHeightMaps(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_HeightmapWidth;
	texDesc.Height = m_HeightmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_heightMapData[0];
	data.SysMemPitch = m_HeightmapWidth * sizeof(float);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = nullptr;
	device->CreateTexture2D(&texDesc, &data, &hmapTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view
	device->CreateShaderResourceView(hmapTex, &srvDesc, &m_HeightMapSRV);

	hmapTex->Release();
}
#pragma endregion

#pragma region Grid Methods
void Terrain::BuildFlatGridVB(ID3D11Device* device)
{
	// Create the vertices for the grid
	std::vector<SimpleVertex> Vertices(m_NumPatchVertRows * m_NumPatchVertCols);

	float halfWidth = 0.5f * m_HeightmapWidth;
	float halfDepth = 0.5f * m_HeightmapHeight;

	float patchWidth = m_HeightmapWidth / (m_NumPatchVertCols - 1);
	float patchDepth = m_HeightmapHeight / (m_NumPatchVertRows - 1);

	float du = 1.0f / (m_NumPatchVertCols - 1);
	float dv = 1.0f / (m_NumPatchVertRows - 1);

	for (unsigned int i = 0; i < m_NumPatchVertRows; i++)
	{
		float z = i * patchDepth - halfDepth;
		for (unsigned int j = 0; j < m_NumPatchVertCols; j++)
		{
			float x = -halfWidth + j * patchWidth;

			Vertices[i * m_NumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);
			Vertices[i * m_NumPatchVertCols + j].TexC.x = j * du;
			Vertices[i * m_NumPatchVertCols + j].TexC.y = i * dv;
		}
	}

	// Create the vertex buffer
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vinitData = {};
	vinitData.pSysMem = Vertices.data();
	device->CreateBuffer(&vbd, &vinitData, &m_TriFlatGridVB);
}

void Terrain::BuildFlatGridIB(ID3D11Device* device)
{
	// Create the indices for the grid
	for (unsigned int i = 0; i < m_NumPatchVertRows - 1; i++)
	{
		for (unsigned int j = 0; j < m_NumPatchVertCols - 1; j++)
		{
			int topleft = i * m_NumPatchVertCols + j;
			int topright = i * m_NumPatchVertCols + (j + 1);
			int bottomleft = (i + 1) * m_NumPatchVertCols + j;
			int bottomright = (i + 1) * m_NumPatchVertCols + (j + 1);

			m_indices.push_back(topright);
			m_indices.push_back(bottomleft);
			m_indices.push_back(bottomright);

			m_indices.push_back(topleft);
			m_indices.push_back(bottomleft);
			m_indices.push_back(topright);
		}
	}

	// Create the index buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * m_indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinitData = {};
	iinitData.pSysMem = &m_indices[0];
	device->CreateBuffer(&ibd, &iinitData, &m_TriFlatGridIB);
}
#pragma endregion