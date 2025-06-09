// Include{s}
#include "DX11Framework.h"
#define RETURNFAIL(x) if(FAILED(x)) return x;

#pragma region Globals
// Some global variables to share between the framework and the window loop
int g_mouseX;
int g_mouseY;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_mouseWheelUP = false;
bool g_mouseWheelDOWN = false;
#pragma endregion

#pragma region Window Loop
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
		// Get mouse position
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);

		// Update mouse position
		g_mouseX = pt.x;
		g_mouseY = pt.y;

		break;

	case WM_MOUSEWHEEL:
	{
		int scrollDirection = GET_WHEEL_DELTA_WPARAM(wParam);

		if (scrollDirection > 0)
		{
			// Handle scroll up
			g_mouseWheelUP = true;
			g_mouseWheelDOWN = false;
		}
		else
		{
			// Handle scroll down
			g_mouseWheelDOWN = true;
			g_mouseWheelUP = false;
		}

		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	// YOU WILL BE MAXIMIZED
	ShowWindow(hWnd, SW_MAXIMIZE);
	ShowCursor(FALSE);

	return message;
}
#pragma endregion

#pragma region Initialization Methods
HRESULT DX11Framework::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	HRESULT hr = S_OK;

	// Initialize application using class methods

	hr = CreateWindowHandle(hInstance, nCmdShow);
	if (FAILED(hr)) return E_FAIL;

	hr = CreateD3DDevice();
	if (FAILED(hr)) return E_FAIL;

	hr = CreateSwapChainAndFrameBuffer();
	if (FAILED(hr)) return E_FAIL;

	hr = InitShaders();
	if (FAILED(hr)) return E_FAIL;

	hr = InitVertexIndexBuffers();
	if (FAILED(hr)) return E_FAIL;

	hr = InitPipelineVariables();
	if (FAILED(hr)) return E_FAIL;

	hr = InitRunTimeData();
	if (FAILED(hr)) return E_FAIL;

	return hr;
}

HRESULT DX11Framework::CreateWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
	// Set the window name
	auto windowName = L"RyanLabs Proprietary Real-Time Rendering Framework";

	WNDCLASSW wndClass = {};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1)); // Load the RyanLabs Icon from the resources
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = windowName;

	RegisterClassW(&wndClass);

	// Create the window
	m_windowHandle = CreateWindowExW(0, windowName, windowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
		CW_USEDEFAULT,
		m_windowWidth, m_windowHeight, nullptr, nullptr, hInstance, nullptr);

	return S_OK;
}

HRESULT DX11Framework::CreateD3DDevice()
{
	HRESULT hr = S_OK;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
	};

	ID3D11Device* baseDevice;
	ID3D11DeviceContext* baseDeviceContext;

	DWORD createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | createDeviceFlags, featureLevels,
		ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);
	if (FAILED(hr)) return hr;

	hr = baseDevice->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_device));
	hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext),
		reinterpret_cast<void**>(&m_immediateContext));

	baseDevice->Release();
	baseDeviceContext->Release();

	hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&m_dxgiDevice));
	if (FAILED(hr)) return hr;

	IDXGIAdapter* dxgiAdapter;
	hr = m_dxgiDevice->GetAdapter(&dxgiAdapter);
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&m_dxgiFactory));
	dxgiAdapter->Release();

	return S_OK;
}

HRESULT DX11Framework::CreateSwapChainAndFrameBuffer()
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = 0; // Defer to m_windowWidth
	swapChainDesc.Height = 0; // Defer to m_windowHeight
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //FLIP* modes don't support sRGB backbuffer
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 4;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	hr = m_dxgiFactory->CreateSwapChainForHwnd(m_device, m_windowHandle, &swapChainDesc, nullptr, nullptr,
		&m_swapChain);
	if (FAILED(hr)) return hr;

	ID3D11Texture2D* frameBuffer = nullptr;

	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frameBuffer));
	if (FAILED(hr)) return hr;

	D3D11_RENDER_TARGET_VIEW_DESC framebufferDesc = {};
	framebufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //sRGB render target enables hardware gamma correction
	framebufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = m_device->CreateRenderTargetView(frameBuffer, &framebufferDesc, &m_frameBufferView);

	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	frameBuffer->GetDesc(&depthBufferDesc);

	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	m_device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer);
	m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView);

	frameBuffer->Release();

	return hr;
}

HRESULT DX11Framework::InitInputLayout()
{
	HRESULT hr = S_OK;

	// Set the input layout to receive the complex object data
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
	hr = m_device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT DX11Framework::InitSimpleShaders()
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// Compile the vertex shader
	hr = D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0",
		dwShaderFlags, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the vertex shader
	hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) return hr;

	// Initialize the input layout
	InitInputLayout();

	// Compile the pixel shader
	hr = D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0",
		dwShaderFlags, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the pixel shader
	hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT DX11Framework::InitSkyBoxShaders()
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// Compile the vertex shader
	hr = D3DCompileFromFile(L"SkyboxShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0",
		dwShaderFlags, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the vertex shader
	hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
		&m_vertexShaderSkybox);
	if (FAILED(hr)) return hr;

	// Compile the pixel shader
	hr = D3DCompileFromFile(L"SkyboxShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0",
		dwShaderFlags, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the pixel shader
	hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
		&m_pixelShaderSkybox);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT DX11Framework::InitHeightMapShader()
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// Compile the vertex shader
	hr = D3DCompileFromFile(L"HeightmapSampler.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0",
		dwShaderFlags, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the vertex shader
	hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
		&m_vertexShaderHeightmap);
	if (FAILED(hr)) return hr;

	// Reset the input layout for the terrain
	D3D11_INPUT_ELEMENT_DESC terrainInputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
	hr = m_device->CreateInputLayout(terrainInputElementDesc, ARRAYSIZE(terrainInputElementDesc),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), &m_terrainInputLayout);
	if (FAILED(hr)) return hr;

	// Compile the pixel shader
	hr = D3DCompileFromFile(L"HeightmapSampler.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0",
		dwShaderFlags, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(m_windowHandle, static_cast<char*>(errorBlob->GetBufferPointer()), nullptr, ERROR);
		if (errorBlob) errorBlob->Release();
		return hr;
	}

	// Create the pixel shader
	hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
		&m_pixelShaderHeightmap);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT DX11Framework::InitShaders()
{
	HRESULT hr = S_OK;

	// Create the shaders on separate threads
	std::thread simpleShaderThread(&DX11Framework::InitSimpleShaders, this);
	std::thread skyBoxShaderThread(&DX11Framework::InitSkyBoxShaders, this);
	std::thread heightMapShaderThread(&DX11Framework::InitHeightMapShader, this);

	// Wait for the threads to finish
	simpleShaderThread.join();
	skyBoxShaderThread.join();
	heightMapShaderThread.join();

	return hr;
}

HRESULT DX11Framework::InitHardCodedObjects()
{
	HRESULT hr = S_OK;

	// Set the vertex buffer for the cube
	SimpleVertex CubeVertexData[] =
	{
		//Position                                           //Normal               // Texture
		{XMFLOAT3(-1.00f, 1.00f, -1.00), XMFLOAT3(-1.00f, 1.00f, -1.00), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.00f, 1.00f, -1.00), XMFLOAT3(1.00f, 1.00f, -1.00), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(-1.00f, -1.00f, -1.00), XMFLOAT3(-1.00f, -1.00f, -1.00), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(1.00f, -1.00f, -1.00), XMFLOAT3(1.00f, -1.00f, -1.00), XMFLOAT2(1.0f, 1.0f)},

		//Position                                           //Normal                 // Texture
		{XMFLOAT3(-1.00f, 1.00f, 1.00), XMFLOAT3(-1.00f, 1.00f, 1.00), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(1.00f, 1.00f, 1.00), XMFLOAT3(1.00f, 1.00f, 1.00), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(-1.00f, -1.00f, 1.00), XMFLOAT3(-1.00f, -1.00f, 1.00), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(1.00f, -1.00f, 1.00), XMFLOAT3(1.00f, -1.00f, 1.00), XMFLOAT2(0.0f, 1.0f)},
	};

	// Set the vertex buffer for the pyramid
	SimpleVertex PyramidVertexData[] =
	{
		{XMFLOAT3(0.00f, 1.00f, 0.00), XMFLOAT3(0.00f, 1.00f, 0.00), XMFLOAT2(0.0f, 0.0f)},

		{XMFLOAT3(1.00f, -1.00f, -1.00), XMFLOAT3(1.00f, -1.00f, -1.00), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(-1.00f, -1.00f, -1.00), XMFLOAT3(-1.00f, -1.00f, -1.00), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(-1.00f, -1.00f, 1.00), XMFLOAT3(-1.00f, -1.00f, 1.00), XMFLOAT2(-1.0f, 0.0f)},
		{XMFLOAT3(1.00f, -1.00f, 1.00), XMFLOAT3(1.00f, -1.00f, 1.00), XMFLOAT2(-1.0f, -1.0f)},
	};

	D3D11_BUFFER_DESC pyramidVertexBufferDesc = {};
	pyramidVertexBufferDesc.ByteWidth = sizeof(PyramidVertexData);
	pyramidVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	pyramidVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA pyramidVertexData = { PyramidVertexData };

	// Create the vertex buffer for the pyramid
	hr = m_device->CreateBuffer(&pyramidVertexBufferDesc, &pyramidVertexData, &m_pyramidVertexBuffer);
	if (FAILED(hr)) return hr;

	D3D11_BUFFER_DESC vertexBufferDesc1 = {};
	vertexBufferDesc1.ByteWidth = sizeof(CubeVertexData);
	vertexBufferDesc1.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc1.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData1 = { CubeVertexData };

	// Create the vertex buffer for the cube
	hr = m_device->CreateBuffer(&vertexBufferDesc1, &vertexData1, &m_vertexBuffer);
	if (FAILED(hr)) return hr;

	// Set the index buffer for the cube
	WORD IndexData[] =
	{
		//Indices
		0, 1, 2,
		2, 1, 3,

		6, 5, 4,
		7, 5, 6,

		1, 5, 3,
		5, 7, 3,

		4, 0, 6,
		6, 0, 2,

		4, 5, 0,
		0, 5, 1,

		2, 7, 6,
		3, 7, 2,
	};

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(IndexData);
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = { IndexData };

	// Create the index buffer for the cube
	hr = m_device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr)) return hr;

	WORD PyramidIndexData[] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 1,

		1, 2, 3,
		3, 4, 1
	};

	D3D11_BUFFER_DESC PyramidindexBufferDesc = {};
	PyramidindexBufferDesc.ByteWidth = sizeof(PyramidIndexData);
	PyramidindexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	PyramidindexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA pyramidindexData = { PyramidIndexData };

	// Create the index buffer for the pyramid
	hr = m_device->CreateBuffer(&PyramidindexBufferDesc, &pyramidindexData, &m_pyramidIndexBuffer);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT DX11Framework::InitVertexIndexBuffers()
{
	HRESULT hr = S_OK;

	// Start the thread to create the hard coded objects
	std::thread hardCodedObjectsThread(&DX11Framework::InitHardCodedObjects, this);

	// Create the terrain object
	m_terrain = new Terrain(m_device);

	// Wait for the thread to finish
	hardCodedObjectsThread.join();

	return hr;
}

HRESULT DX11Framework::InitPipelineVariables()
{
	HRESULT hr = S_OK;

	//Input Assembler
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_immediateContext->IASetInputLayout(m_inputLayout);

	////////////////////////Rasterizer//////////////////////////

	// Fill
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;

	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	// Create the rasterizer state for fill
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_fillState);
	if (FAILED(hr)) return hr;

	// WireFrame
	D3D11_RASTERIZER_DESC wireframeDesc = {};
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_NONE;

	hr = m_device->CreateRasterizerState(&wireframeDesc, &m_wireframeState);
	if (FAILED(hr)) return hr;

	// No backface culling
	D3D11_RASTERIZER_DESC nobackfacecullDesc = {};
	nobackfacecullDesc.FillMode = D3D11_FILL_SOLID;
	nobackfacecullDesc.CullMode = D3D11_CULL_NONE;

	// Create the rasterizer state for no backface culling
	hr = m_device->CreateRasterizerState(&nobackfacecullDesc, &m_dontCullBackState);
	if (FAILED(hr)) return hr;

	////////////////////////Rasterizer//////////////////////////

	//Viewport Values
	m_viewport = { 0.0f, 0.0f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), 0.0f, 1.0f };
	m_immediateContext->RSSetViewports(1, &m_viewport);

	//Constant Buffer
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//
	hr = m_device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr)) { return hr; }

	m_immediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	///////////////////////////////////////////////////////////

	D3D11_SAMPLER_DESC bilinearSamplerdesc = {};
	bilinearSamplerdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearSamplerdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearSamplerdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	bilinearSamplerdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	bilinearSamplerdesc.MaxLOD = 1;
	bilinearSamplerdesc.MinLOD = 0;

	// Create the bilinear sampler state
	hr = m_device->CreateSamplerState(&bilinearSamplerdesc, &m_bilinearSamplerState);
	if (FAILED(hr)) return hr;

	D3D11_BLEND_DESC transparencyDesc = {};

	D3D11_RENDER_TARGET_BLEND_DESC rtdb = {};
	rtdb.BlendEnable = true;
	rtdb.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtdb.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtdb.BlendOp = D3D11_BLEND_OP_ADD;
	rtdb.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtdb.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtdb.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtdb.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	transparencyDesc.AlphaToCoverageEnable = false;
	transparencyDesc.RenderTarget[0] = rtdb;

	// Create the transparency blend state
	m_device->CreateBlendState(&transparencyDesc, &m_transparency);

	D3D11_DEPTH_STENCIL_DESC dsDescSkybox = {};
	dsDescSkybox.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDescSkybox.DepthEnable = true;
	dsDescSkybox.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	// Create the depth stencil state for the skybox
	m_device->CreateDepthStencilState(&dsDescSkybox, &m_depthStencilSkybox);

	// Text and Sprite Initializations (Yippee)
	m_spriteBatch = std::make_unique<SpriteBatch>(m_immediateContext);
	m_spriteFont = std::make_unique<SpriteFont>(m_device, L"Fonts\\Font1.spritefont");

	m_cbData.LightON = 1;

	return S_OK;
}

void DX11Framework::LoadLightVariables()
{
	// Using nlohmann.json Library (Not Mine!)
	// Using a JSON file to store the light variables
	std::ifstream file("JSON Files\\Light Variables.json");

	// Check if the file is open
	if (!file.is_open())
	{
		std::cerr << "Failed to open JSON file." << std::endl;
	}

	// Load the JSON file
	file >> m_lightVariables;

	m_lightDir = XMFLOAT3(
		m_lightVariables["lightDirection"]["x"].get<float>(),
		m_lightVariables["lightDirection"]["y"].get<float>(),
		m_lightVariables["lightDirection"]["z"].get<float>()
	);
	m_diffuseMaterial = XMFLOAT4(
		m_lightVariables["diffuse"]["material"]["r"].get<float>(),
		m_lightVariables["diffuse"]["material"]["g"].get<float>(),
		m_lightVariables["diffuse"]["material"]["b"].get<float>(),
		m_lightVariables["diffuse"]["material"]["a"].get<float>()
	);
	m_diffuseLight = XMFLOAT4(
		m_lightVariables["diffuse"]["light"]["r"].get<float>(),
		m_lightVariables["diffuse"]["light"]["g"].get<float>(),
		m_lightVariables["diffuse"]["light"]["b"].get<float>(),
		m_lightVariables["diffuse"]["light"]["a"].get<float>()
	);
	m_ambientMaterial = XMFLOAT4(
		m_lightVariables["ambient"]["material"]["r"].get<float>(),
		m_lightVariables["ambient"]["material"]["g"].get<float>(),
		m_lightVariables["ambient"]["material"]["b"].get<float>(),
		m_lightVariables["ambient"]["material"]["a"].get<float>()
	);
	m_ambientLight = XMFLOAT4(
		m_lightVariables["ambient"]["light"]["r"].get<float>(),
		m_lightVariables["ambient"]["light"]["g"].get<float>(),
		m_lightVariables["ambient"]["light"]["b"].get<float>(),
		m_lightVariables["ambient"]["light"]["a"].get<float>()
	);
	m_specularMaterial = XMFLOAT4(
		m_lightVariables["specular"]["material"]["r"].get<float>(),
		m_lightVariables["specular"]["material"]["g"].get<float>(),
		m_lightVariables["specular"]["material"]["b"].get<float>(),
		m_lightVariables["specular"]["material"]["a"].get<float>()
	);
	m_specularLight = XMFLOAT4(
		m_lightVariables["specular"]["light"]["r"].get<float>(),
		m_lightVariables["specular"]["light"]["g"].get<float>(),
		m_lightVariables["specular"]["light"]["b"].get<float>(),
		m_lightVariables["specular"]["light"]["a"].get<float>()
	);
	m_specPower = m_lightVariables["specular"]["power"].get<float>();

	// Close the file
	file.close();

	// Set the constant buffer data
	m_cbData.DiffuseLight = m_diffuseLight;
	m_cbData.DiffuseMaterial = m_diffuseMaterial;
	m_cbData.LightDir = m_lightDir;
	m_cbData.AmbientLight = m_ambientLight;
	m_cbData.AmbientMaterial = m_ambientMaterial;
	m_cbData.SpecularLight = m_specularLight;
	m_cbData.SpecularMaterial = m_specularMaterial;
	m_cbData.CameraPosition = m_startingCameraPosition;
	m_cbData.SpecPower = m_specPower;
}

void DX11Framework::LoadSceneCameraVariables()
{
	std::ifstream file("JSON Files\\Scene Camera Variables.json");

	// Check if the file is open
	if (!file.is_open())
	{
		std::cerr << "Failed to open JSON file." << std::endl;
	}

	// Load the JSON file
	file >> m_sceneCameraVariables;

	//Camera
	float aspect = m_viewport.Width / m_viewport.Height;

	//////////////////////////////////////

	m_debugCamera.SetPosition(
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Position"]["z"].get<
		float>());

	m_debugCamera.SetRotation(
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["Rotation"]["z"].get<
		float>());

	m_debugCamera.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["DebugCamera"]["ProjectionValues"]["far"].get<float>());
	m_startingCameraPosition = XMFLOAT3(m_debugCamera.GetPosition());

	//////////////////////////////////////

	m_camera1.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Position"]["z"].get<float>());

	m_camera1.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["Rotation"]["z"].get<float>());

	m_camera1.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraOne"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera2.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Position"]["z"].get<float>());

	m_camera2.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["Rotation"]["z"].get<float>());

	m_camera2.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraTwo"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera3.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Position"]["z"].get<float>());

	m_camera3.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["Rotation"]["z"].get<float>());

	m_camera3.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraThree"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera4.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Position"]["z"].get<float>());

	m_camera4.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["Rotation"]["z"].get<float>());

	m_camera4.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFour"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera5.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Position"]["z"].get<float>());

	m_camera5.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["Rotation"]["z"].get<float>());

	m_camera5.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraFive"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera6.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Position"]["z"].get<float>());

	m_camera6.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["Rotation"]["z"].get<float>());

	m_camera6.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSix"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera7.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Position"]["z"].get<float>());

	m_camera7.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["Rotation"]["z"].get<float>());

	m_camera7.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraSeven"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera8.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Position"]["z"].get<float>());

	m_camera8.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["Rotation"]["z"].get<float>());

	m_camera8.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraEight"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	m_camera9.SetPosition(m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Position"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Position"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Position"]["z"].get<float>());

	m_camera9.SetRotation(m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Rotation"]["x"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Rotation"]["y"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["Rotation"]["z"].get<float>());

	m_camera9.SetProjectionValues(
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["ProjectionValues"]["fov"].get<float>(), aspect,
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["ProjectionValues"]["near"].get<float>(),
		m_sceneCameraVariables["SceneCameraVariables"]["CameraNine"]["ProjectionValues"]["far"].get<float>());

	//////////////////////////////////////

	// Close the file
	file.close();

	// Set the View Matrix
	XMStoreFloat4x4(&m_View, m_debugCamera.GetViewMatrix());
	// Set the Projection Matrix
	XMStoreFloat4x4(&m_Projection, m_debugCamera.GetProjectionMatrix());
}

HRESULT DX11Framework::LoadUI(HRESULT hr)
{
	// Load the UI Textures
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Crate_COLOR.dds", nullptr, &m_crateTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\RyanLabs.dds", nullptr, &m_ryanlabsTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Wkey.dds", nullptr, &m_wTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Akey.dds", nullptr, &m_aTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Skey.dds", nullptr, &m_sTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Dkey.dds", nullptr, &m_dTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Qkey.dds", nullptr, &m_qTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Ekey.dds", nullptr, &m_eTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\UpKey.dds", nullptr, &m_upTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\LeftKey.dds", nullptr, &m_leftTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\RightKey.dds", nullptr, &m_rightTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\DownKey.dds", nullptr, &m_downTexture);
	hr = CreateDDSTextureFromFile(m_device, L"Textures\\Skybox.dds", nullptr, &m_skyboxTexture);

	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}

HRESULT DX11Framework::InitRunTimeData()
{
	HRESULT hr = S_OK;

	// Start the runtime data threads
	std::thread loadlightThread(&DX11Framework::LoadLightVariables, this);
	std::thread loadcamerasThread(&DX11Framework::LoadSceneCameraVariables, this);
	std::thread loadUIThread(&DX11Framework::LoadUI, this, hr);
	std::thread loadobjectsThread(&DX11Framework::LoadGameObjectDataFromSceneJSON, this);

	// Set some default values for the constant buffer
	m_cbData.hasTexture = 1;
	m_cbData.pixelationAmount = 20.0f;

	// Wait for the threads to finish
	loadcamerasThread.join();
	loadlightThread.join();
	loadUIThread.join();
	loadobjectsThread.join();

	return S_OK;
}

void DX11Framework::LoadGameObjectDataFromSceneJSON()
{
	// Load some non-scene graph objects (Skybox, Main Menu Object)
	m_mainMenuObject = new GameObject(m_device, "Test models\\Made In Blender\\donut.obj", L"NULL", XMFLOAT3(0, 1, -11),
		XMFLOAT3(0, 0, 0), XMFLOAT3(1.45f, 1.45f, 1.45f), 0, "mainmenuobject");

	m_skybox = new GameObject(m_device, "OBJ's\\InvertedCube.obj", L"Textures\\skybox.dds",
		XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1000, 1000, 1000), 0, "skybox");

	std::ifstream file("JSON Files\\Scene Graph.json");

	// Check if the file is open
	if (!file.is_open())
	{
		std::cerr << "Failed to open JSON file." << std::endl;
		return;
	}

	// Load the JSON file
	file >> m_sceneData;

	for (int i = 0; i < m_sceneData["GameObjects"].size(); i++)
	{
		// Load the game objects
		LoadGameObject(i);
	}

	file.close();
}

// I'm surprised I got away with going this far without needing to use this function
std::wstring StringToWString(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

void DX11Framework::LoadGameObject(int i)
{
	// Load the game objects paths
	auto tempOBJfilepath = m_sceneData["GameObjects"][i]["OBJfilepath"].get<std::string>();
	auto tempTexturefilepath = m_sceneData["GameObjects"][i]["TEXfilepath"].get<std::string>();

	// Load the game objects positions, rotations, scales, id's and names
	XMFLOAT3 tempPosition(
		m_sceneData["GameObjects"][i]["position"]["x"].get<float>(),
		m_sceneData["GameObjects"][i]["position"]["y"].get<float>(),
		m_sceneData["GameObjects"][i]["position"]["z"].get<float>()
	);

	XMFLOAT3 tempRotation(
		m_sceneData["GameObjects"][i]["rotation"]["x"].get<float>(),
		m_sceneData["GameObjects"][i]["rotation"]["y"].get<float>(),
		m_sceneData["GameObjects"][i]["rotation"]["z"].get<float>()
	);

	XMFLOAT3 tempScale(
		m_sceneData["GameObjects"][i]["scale"]["x"].get<float>(),
		m_sceneData["GameObjects"][i]["scale"]["y"].get<float>(),
		m_sceneData["GameObjects"][i]["scale"]["z"].get<float>()
	);

	int tempID = m_sceneData["GameObjects"][i]["id"].get<int>();
	auto tempName = m_sceneData["GameObjects"][i]["Name"].get<std::string>();

	// Convert the strings to wstrings
	std::wstring tempTextureWfilepath = StringToWString(tempTexturefilepath);

	// Create the game object
	auto tempGameObject = new GameObject(m_device, tempOBJfilepath.c_str(), tempTextureWfilepath.c_str(), tempPosition,
		tempRotation, tempScale, tempID, tempName);

	// Fancy C++ Vector
	m_gameObjects.push_back(tempGameObject);
}
#pragma endregion

#pragma region Draw Methods
void DX11Framework::DrawKey(KeyState state, ID3D11ShaderResourceView* texture, XMFLOAT2 position) const
{
	// Draw the key based on the state
	switch (state)
	{
	case Key_DOWN:
		m_spriteBatch->Draw(texture, position, nullptr, Colors::Green, 0.0f, XMFLOAT2(0, 0), 0.5f);
		break;
	case Key_UP:
		m_spriteBatch->Draw(texture, position, nullptr, Colors::White, 0.0f, XMFLOAT2(0, 0), 0.5f);
		break;
	default: break;
	}
}

void DX11Framework::RenderTransparent() const
{
	// Set the blend state for transparency
	m_immediateContext->OMSetBlendState(m_transparency, m_transparencyBlendFactor, 0xffffffff);
}

void DX11Framework::RenderOpaque() const
{
	// Set the blend state for opaque objects
	m_immediateContext->OMSetBlendState(nullptr, m_blendfactor, 0xffffffff);
}

void DX11Framework::DrawUI()
{
	// Draw the UI, if the text rendering is enabled
	if (m_textRendering)
	{
		// Begin the sprite batch
		m_spriteBatch->Begin();

		// RyanLabs Logo
		m_spriteBatch->Draw(m_ryanlabsTexture, XMFLOAT2(1800, 0), nullptr, Colors::White, 0.0f, XMFLOAT2(0, 0), 0.02f);

		// Debug Camera Controls
		if (m_cameraNumActive == 0)
		{
			DrawKey(m_WState, m_wTexture, XMFLOAT2(1300, 790));
			DrawKey(m_AState, m_aTexture, XMFLOAT2(1190, 900));
			DrawKey(m_SState, m_sTexture, XMFLOAT2(1300, 900));
			DrawKey(m_DState, m_dTexture, XMFLOAT2(1410, 900));
			DrawKey(m_QState, m_qTexture, XMFLOAT2(1190, 790));
			DrawKey(m_EState, m_eTexture, XMFLOAT2(1410, 790));
			DrawKey(m_UPState, m_upTexture, XMFLOAT2(1650, 790));
			DrawKey(m_LEFTState, m_leftTexture, XMFLOAT2(1540, 900));
			DrawKey(m_DOWNState, m_downTexture, XMFLOAT2(1650, 900));
			DrawKey(m_RIGHTState, m_rightTexture, XMFLOAT2(1760, 900));
		}

		// Texture UI
		if (m_cbData.hasTexture == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F1 - Textures: ON", XMFLOAT2(0, 0), Colors::Green);
		}
		else if (m_cbData.hasTexture == 0)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F1 - Textures: OFF", XMFLOAT2(0, 0), Colors::Red);
		}

		// Lighting UI
		if (m_cbData.LightON == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F2 - Lighting: ON", XMFLOAT2(500, 0), Colors::Green);
		}
		else
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F2 - Lighting: OFF", XMFLOAT2(500, 0), Colors::Red);
		}

		// Fill Mode UI
		if (m_fill)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F3 - Fill Mode: ON", XMFLOAT2(1000, 0), Colors::Green);
		}
		else
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F3 - Fill Mode: OFF", XMFLOAT2(1000, 0), Colors::Red);
		}

		// Back Face Culling UI
		if (m_nobackfaceCulling)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F6 - Back Face Culling Mode: OFF", XMFLOAT2(1515, 350),
				Colors::Red);
		}
		else
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F6 - Back Face Culling Mode: ON", XMFLOAT2(1515, 350),
				Colors::Green);
		}

		// Plane Rotation UI
		if (m_rotate)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F4 - Scene 1 Plane Rotation: ON", XMFLOAT2(1400, 0),
				Colors::Green);
		}
		else
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F4 - Scene 1 Plane Rotation: OFF", XMFLOAT2(1400, 0),
				Colors::Red);
		}

		// Wave Filter Y UI
		if (m_cbData.waveFilter == 0)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F7 - Wave Filter Y: OFF", XMFLOAT2(1515, 400), Colors::Red);
		}
		else if (m_cbData.waveFilter == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F7 - Wave Filter Y: ON", XMFLOAT2(1515, 400),
				Colors::Green);
		}

		// Wave Filter X UI
		if (m_cbData.waveFilterX == 0)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F8 - Wave Filter X: OFF", XMFLOAT2(1515, 450), Colors::Red);
		}
		else if (m_cbData.waveFilterX == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F8 - Wave Filter X: ON", XMFLOAT2(1515, 450),
				Colors::Green);
		}

		// Mouse Mode UI
		if (m_mouseMode == false)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F9 - Mouse Features: OFF", XMFLOAT2(1515, 500),
				Colors::Red);
		}
		else if (m_mouseMode == true)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F9 - Mouse Features: ON", XMFLOAT2(1515, 500),
				Colors::Green);
		}

		// Pixelation Mode UI
		if (m_cbData.pixelateFilter == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"TAB - Pixelation Mode: ON", XMFLOAT2(1515, 550),
				Colors::Green);
		}
		else if (m_cbData.pixelateFilter == 0)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"TAB - Pixelation Mode: OFF", XMFLOAT2(1515, 550),
				Colors::Red);
		}

		// General UI Information
		m_spriteFont->DrawString(m_spriteBatch.get(), L"R - Reset Camera", XMFLOAT2(1515, 300), Colors::Purple);

		// If UI rendering is on, you can see this you dummy.
		m_spriteFont->DrawString(m_spriteBatch.get(), L"F5 - UI Rendering: ON", XMFLOAT2(0, 200), Colors::Green);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"ESC - Main Menu", XMFLOAT2(0, 250), Colors::Red);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 0 - Debug Camera", XMFLOAT2(0, 400), Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 1 - Plane Interior", XMFLOAT2(0, 450),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 2 - Plane Exterior", XMFLOAT2(0, 500), Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 3 - Fish Tank Interior", XMFLOAT2(0, 550),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 4 - Gooch Shading Room", XMFLOAT2(0, 600),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 5 - Milk (Fake Shadow-mapping)", XMFLOAT2(0, 650),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 6 - Well", XMFLOAT2(0, 700), Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 7 - Glassware (Transparency)", XMFLOAT2(0, 750),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 8 - Car (Pixelation Shader)", XMFLOAT2(0, 800),
			Colors::White);

		m_spriteFont->DrawString(m_spriteBatch.get(), L"Numpad 9 - Terrain Generation (Broken)", XMFLOAT2(0, 850),
			Colors::White);

		// Gooch Shading UI
		if (m_cbData.goochShading == 0)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F11 - Gooch Shading: OFF", XMFLOAT2(1515, 600),
				Colors::Red);
		}
		else if (m_cbData.goochShading == 1)
		{
			m_spriteFont->DrawString(m_spriteBatch.get(), L"F11 - Gooch Shading: ON", XMFLOAT2(1515, 600),
				Colors::Green);
		}

		// Determine which camera is active and display it
		switch (m_cameraNumActive)
		{
		case 0:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Debug Camera Active", XMFLOAT2(0, 300), Colors::Purple);
			break;

		case 1:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 1 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 2:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 3 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 3:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 2 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 4:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 4 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 5:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 5 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 6:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 6 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 7:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 7 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 8:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 8 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		case 9:
			m_spriteFont->DrawString(m_spriteBatch.get(), L"Camera 9 Active", XMFLOAT2(0, 300), Colors::White);
			break;

		default: break;
		}

		// Draw the active UI
		if (m_cameraNumActive == 0)
		{
			std::string countStr = "Time Running: " + std::to_string(m_timeRunning);
			m_spriteFont->DrawString(m_spriteBatch.get(), countStr.c_str(), XMFLOAT2(0, 1010), Colors::Purple);

			std::string cameraPosStrX = "X POS: " + std::to_string(m_debugCamera.GetPosition().x);
			std::string cameraPosStrY = "Y POS: " + std::to_string(m_debugCamera.GetPosition().y);
			std::string cameraPosStrZ = "Z POS: " + std::to_string(m_debugCamera.GetPosition().z);

			std::string cameraPos = cameraPosStrX + " " + cameraPosStrY + " " + cameraPosStrZ;

			std::string cameraRotStrX = "X ROT: " + std::to_string(m_debugCamera.GetRotation().x);
			std::string cameraRotStrY = "Y ROT: " + std::to_string(m_debugCamera.GetRotation().y);

			std::string cameraRot = cameraRotStrX + " " + cameraRotStrY;

			m_spriteFont->DrawString(m_spriteBatch.get(), cameraRot.c_str(), XMFLOAT2(0, 980), Colors::Purple);
			m_spriteFont->DrawString(m_spriteBatch.get(), cameraPos.c_str(), XMFLOAT2(0, 960), Colors::Purple);

			std::string mouseposXStr = "Mouse X: " + std::to_string(g_mouseX);
			std::string mouseposYStr = "Mouse Y: " + std::to_string(g_mouseY);

			m_spriteFont->DrawString(m_spriteBatch.get(), mouseposXStr.c_str(), XMFLOAT2(0, 920), Colors::Purple);
			m_spriteFont->DrawString(m_spriteBatch.get(), mouseposYStr.c_str(), XMFLOAT2(0, 940), Colors::Purple);
		}

		// End the sprite batch
		m_spriteBatch->End();

		// Reset the state of the keys
		m_WState = Key_UP;
		m_AState = Key_UP;
		m_SState = Key_UP;
		m_DState = Key_UP;
		m_UPState = Key_UP;
		m_EState = Key_UP;
		m_QState = Key_UP;
		m_LEFTState = Key_UP;
		m_DOWNState = Key_UP;
		m_RIGHTState = Key_UP;
	}
}

void DX11Framework::DrawMainMenu() const
{
	// Begin the sprite batch
	m_spriteBatch->Begin();

	// Draw the main menu text
	m_spriteFont->DrawString(m_spriteBatch.get(), L"RyanLabs Proprietary Real-Time Rendering Framework", XMFLOAT2(0, 0),
		Colors::GhostWhite);

	m_spriteFont->DrawString(m_spriteBatch.get(), L"Arrow Keys to Select / Enter to Confirm", XMFLOAT2(0, 50),
		Colors::GhostWhite);

	m_spriteBatch->Draw(m_ryanlabsTexture, XMFLOAT2(1800, 0), nullptr, Colors::White, 0.0f, XMFLOAT2(0, 0), 0.02f);

	// Draw the main menu options
	if (m_playVideo)
	{
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Start", XMFLOAT2(0, 100), Colors::Red);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Exit", XMFLOAT2(100, 100), Colors::Red);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Play Tutorial Video", XMFLOAT2(200, 100), Colors::Green);
	}
	else if (m_start)
	{
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Start", XMFLOAT2(0, 100), Colors::Green);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Exit", XMFLOAT2(100, 100), Colors::Red);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Play Tutorial Video", XMFLOAT2(200, 100), Colors::Red);
	}
	else if (!m_start)
	{
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Start", XMFLOAT2(0, 100), Colors::Red);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Exit", XMFLOAT2(100, 100), Colors::Green);
		m_spriteFont->DrawString(m_spriteBatch.get(), L"Play Tutorial Video", XMFLOAT2(200, 100), Colors::Red);
	}

	// End the sprite batch
	m_spriteBatch->End();
}

void DX11Framework::Draw()
{
	// Create the stride and offset
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Reset some of the pipeline variables
	UpdatePipelineVariables(stride, offset);

	// Render the skybox
	RenderSkybox(stride, offset);

	if (m_mainMenu == false)
	{
		// Draw the first cube
		m_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_World));
		D3D11_MAPPED_SUBRESOURCE mappedSubresourceCube1{};
		m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceCube1);
		memcpy(mappedSubresourceCube1.pData, &m_cbData, sizeof(m_cbData));
		m_immediateContext->Unmap(m_constantBuffer, 0);
		m_immediateContext->DrawIndexed(36, 0, 0);
		///////////////////////////////////////////////////////////
		// Draw the second cube
		m_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_World2));
		D3D11_MAPPED_SUBRESOURCE mappedSubresourceCube2{};
		m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceCube2);
		memcpy(mappedSubresourceCube2.pData, &m_cbData, sizeof(m_cbData));
		m_immediateContext->Unmap(m_constantBuffer, 0);
		m_immediateContext->DrawIndexed(36, 0, 0);
		///////////////////////////////////////////////////////////
		// Draw the third cube
		m_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_World3));
		D3D11_MAPPED_SUBRESOURCE mappedSubresourceCube3{};
		m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceCube3);
		memcpy(mappedSubresourceCube3.pData, &m_cbData, sizeof(m_cbData));
		m_immediateContext->Unmap(m_constantBuffer, 0);
		m_immediateContext->DrawIndexed(36, 0, 0);
		///////////////////////////////////////////////////////////
		// Draw the pyramid
		m_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_Pyramid));
		D3D11_MAPPED_SUBRESOURCE mappedSubresourcePyramid{};
		m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourcePyramid);
		memcpy(mappedSubresourcePyramid.pData, &m_cbData, sizeof(m_cbData));
		m_immediateContext->Unmap(m_constantBuffer, 0);
		m_immediateContext->IASetVertexBuffers(0, 1, &m_pyramidVertexBuffer, &stride, &offset);
		m_immediateContext->IASetIndexBuffer(m_pyramidIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_immediateContext->DrawIndexed(18, 0, 0);

		// Draw the game objects
		for (const auto& gameObject : m_gameObjects)
		{
			// If the game object is transparent, render it as such
			if (gameObject->m_Name == "Glass")
			{
				RenderTransparent();
				gameObject->Draw(m_cbData, m_immediateContext, m_constantBuffer);
				RenderOpaque();
			}
			else
			{
				gameObject->Draw(m_cbData, m_immediateContext, m_constantBuffer);
			}
		}

		// Render the terrain
		RenderTerrain(stride, offset);

		///////////////////////////////////////////////////////////
		// UI Drawing
		// Using DirectX11 Toolkit by Microsoft Library for SpriteBatching and SpriteFonts (Not Mine!)
		// Designed for 1080p resolution
		DrawUI();
	}
	else
	{
		// Set the main menu to render transparent
		RenderTransparent();

		m_mainMenuObject->Draw(m_cbData, m_immediateContext, m_constantBuffer);

		// Set the main menu to render opaque
		RenderOpaque();

		// Draw the main menu UI
		DrawMainMenu();
	}

	// Present Backbuffer to screen
	m_swapChain->Present(0, 0);
}

// Render the terrain (Should be moved into the class)
void DX11Framework::RenderTerrain(UINT stride, UINT offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};

	m_cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_terrain->m_matrix));

	m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &m_cbData, sizeof(m_cbData));
	m_immediateContext->Unmap(m_constantBuffer, 0);

	m_immediateContext->VSSetShader(m_vertexShaderHeightmap, nullptr, 0);
	m_immediateContext->PSSetShader(m_pixelShaderHeightmap, nullptr, 0);

	m_immediateContext->IASetInputLayout(m_terrainInputLayout);

	m_immediateContext->VSSetShaderResources(0, 1, &m_terrain->m_HeightMapSRV);
	m_immediateContext->VSSetSamplers(0, 1, &m_bilinearSamplerState);
	m_immediateContext->PSSetShaderResources(0, 1, &m_terrain->m_HeightMapSRV);
	m_immediateContext->PSSetSamplers(0, 1, &m_bilinearSamplerState);

	m_immediateContext->IASetVertexBuffers(0, 1, &m_terrain->m_TriFlatGridVB, &stride, &offset);
	m_immediateContext->IASetIndexBuffer(m_terrain->m_TriFlatGridIB, DXGI_FORMAT_R32_UINT, 0);

	m_immediateContext->DrawIndexed(m_terrain->m_indices.size(), 0, 0);
}

// Render the skybox
void DX11Framework::RenderSkybox(UINT stride, UINT offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};

	m_immediateContext->VSSetShader(m_vertexShaderSkybox, nullptr, 0);
	m_immediateContext->PSSetShader(m_pixelShaderSkybox, nullptr, 0);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilSkybox, 1);
	m_immediateContext->PSSetShaderResources(0, 1, &m_skyboxTexture);
	m_immediateContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	m_skybox->Draw(m_cbData, m_immediateContext, m_constantBuffer);

	// Move the skybox with the camera
	if (m_debugCamera.GetPosition().z > 0)
	{
		m_cbData.CameraPosition = m_startingCameraPosition;
	}
	else if (m_debugCamera.GetPosition().z < 0)
	{
		m_cbData.CameraPosition = m_debugCamera.GetPosition();
	}

	m_cbData.View = XMMatrixTranspose(XMLoadFloat4x4(&m_View));
	m_cbData.Projection = XMMatrixTranspose(XMLoadFloat4x4(&m_Projection));

	if (m_fill)
	{
		m_immediateContext->RSSetState(m_fillState);
	}
	else if (!m_fill && !m_nobackfaceCulling)
	{
		m_immediateContext->RSSetState(m_wireframeState);
	}
	else if (!m_fill && m_nobackfaceCulling)
	{
		m_immediateContext->RSSetState(m_dontCullBackState);
	}

	// Write constant buffer data onto GPU
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_immediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_immediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_immediateContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_immediateContext->PSSetShader(m_pixelShader, nullptr, 0);
	m_immediateContext->PSSetSamplers(0, 1, &m_bilinearSamplerState);
	m_immediateContext->PSSetShaderResources(0, 1, &m_crateTexture);
	m_immediateContext->PSSetShaderResources(1, 1, &m_ryanlabsTexture);
	m_immediateContext->RSSetViewports(1, &m_viewport);
	RenderOpaque();
	m_immediateContext->OMSetDepthStencilState(nullptr, 0);
}

#pragma endregion

#pragma region Update Methods

void DX11Framework::HandleDebugMovement(float deltatime)
{
	// Move the camera with the WASD keys
	if (GetAsyncKeyState(87) & 0xFFFF) // W
	{
		m_WState = Key_DOWN;
		m_debugCamera.AddToPosition((m_debugCamera.GetForwardVector() * m_cameraSpeed) * deltatime);
	}
	if (GetAsyncKeyState(83) & 0xFFFF) // S
	{
		m_SState = Key_DOWN;
		m_debugCamera.AddToPosition((m_debugCamera.GetBackVector() * m_cameraSpeed) * deltatime);
	}
	if (GetAsyncKeyState(65) & 0xFFFF) // A
	{
		m_AState = Key_DOWN;
		m_debugCamera.AddToPosition((m_debugCamera.GetLeftVector() * m_cameraSpeed) * deltatime);
	}
	if (GetAsyncKeyState(68) & 0xFFFF) // D
	{
		m_DState = Key_DOWN;
		m_debugCamera.AddToPosition((m_debugCamera.GetRightVector() * m_cameraSpeed) * deltatime);
	}
	if (GetAsyncKeyState(81) & 0xFFFF) // Q
	{
		m_QState = Key_DOWN;
		m_debugCamera.AddToPosition(0.0f, m_cameraSpeed * deltatime, 0.0f);
	}
	if (GetAsyncKeyState(69) & 0xFFFF) // E
	{
		m_EState = Key_DOWN;
		m_debugCamera.AddToPosition(0, -m_cameraSpeed * deltatime, 0);
	}
	if (GetAsyncKeyState(82) & 0xFFFF) // R
	{
		m_cbData.waveFilter = 0;
		m_nobackfaceCulling = false;
		m_fill = true;
		m_cameraNumActive = 0;
		m_cbData.LightON = 1;
		m_cbData.hasTexture = 1;
		m_rotate = false;
		m_fill = true;
		m_textRendering = true;
		m_debugCamera.SetPosition(m_startingCameraPosition.x, m_startingCameraPosition.y, m_startingCameraPosition.z);
		m_debugCamera.SetRotation(0, 0, 0);
	}

	// Rotate the camera with the arrow keys
	if (GetAsyncKeyState(VK_UP) & 0xFFFF)
	{
		m_UPState = Key_DOWN;
		m_debugCamera.AddToRotation(-m_rotationSpeed * deltatime, 0.0f, 0.0f);
	}
	if (GetAsyncKeyState(VK_DOWN) & 0xFFFF)
	{
		m_DOWNState = Key_DOWN;
		m_debugCamera.AddToRotation(m_rotationSpeed * deltatime, 0.0f, 0.0f);
	}
	if (GetAsyncKeyState(VK_LEFT) & 0xFFFF)
	{
		m_LEFTState = Key_DOWN;
		m_debugCamera.AddToRotation(0.0f, -m_rotationSpeed * deltatime, 0.0f);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0xFFFF)
	{
		m_RIGHTState = Key_DOWN;
		m_debugCamera.AddToRotation(0.0f, m_rotationSpeed * deltatime, 0.0f);
	}

	// Activate mouse features
	if (GetAsyncKeyState(VK_F9) & 0x0001)
	{
		if (m_mouseMode)
		{
			m_mouseMode = false;
		}
		else
		{
			m_mouseMode = true;
		}
	}

	if (m_mouseMode)
	{
		// Find the change in mouse position
		float deltaX = (g_mouseX - g_lastMouseX) * (0.1 * deltatime);
		float deltaY = (g_mouseY - g_lastMouseY) * (0.1 * deltatime);

		// Add the change to the camera rotation
		if (abs(g_mouseX - g_lastMouseX) > m_deadZone || abs(g_mouseY - g_lastMouseY) > m_deadZone)
		{
			m_debugCamera.AddToRotation(deltaY, deltaX, 0.0f);

			g_lastMouseX = 960;
			g_lastMouseY = 540;
			SetCursorPos(960, 540);
		}

		// Move the camera with the mouse wheel
		if (g_mouseWheelUP)
		{
			m_debugCamera.AddToPosition(m_debugCamera.GetForwardVector() * 1);
			g_mouseWheelUP = false;
		}
		if (g_mouseWheelDOWN)
		{
			m_debugCamera.AddToPosition(m_debugCamera.GetBackVector() * 1);
			g_mouseWheelDOWN = false;
		}
	}
}

void DX11Framework::UpdatePipelineVariables(UINT stride, UINT offset)
{
	// Present unbinds render target, so rebind and clear at the start of each frame
	float backgroundColor[4] = { 0.025f, 0.025f, 0.095f, 1.0f };
	m_immediateContext->OMSetRenderTargets(1, &m_frameBufferView, m_depthStencilView);
	m_immediateContext->ClearRenderTargetView(m_frameBufferView, backgroundColor);
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (m_debugCamera.GetPosition().z > 0)
	{
		m_cbData.CameraPosition = m_startingCameraPosition;
	}
	else if (m_debugCamera.GetPosition().z < 0)
	{
		m_cbData.CameraPosition = m_debugCamera.GetPosition();
	}

	// Update the active camera
	switch (m_cameraNumActive)
	{
	case 0:
		// Use the debug camera
		XMStoreFloat4x4(&m_View, m_debugCamera.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_debugCamera.GetProjectionMatrix());
		break;

	case 1:
		// Camera 1
		m_debugCamera.SetPosition(m_camera1.GetPosition().x, m_camera1.GetPosition().y, m_camera1.GetPosition().z);
		m_debugCamera.SetRotation(m_camera1.GetRotation().x, m_camera1.GetRotation().y, m_camera1.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera1.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera1.GetProjectionMatrix());
		break;

	case 2:
		// Camera 2
		m_debugCamera.SetPosition(m_camera2.GetPosition().x, m_camera2.GetPosition().y, m_camera2.GetPosition().z);
		m_debugCamera.SetRotation(m_camera2.GetRotation().x, m_camera2.GetRotation().y, m_camera2.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera2.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera2.GetProjectionMatrix());
		break;

	case 3:
		// Camera 3
		m_debugCamera.SetPosition(m_camera3.GetPosition().x, m_camera3.GetPosition().y, m_camera3.GetPosition().z);
		m_debugCamera.SetRotation(m_camera3.GetRotation().x, m_camera3.GetRotation().y, m_camera3.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera3.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera3.GetProjectionMatrix());
		break;

	case 4:
		// Camera 4
		m_debugCamera.SetPosition(m_camera4.GetPosition().x, m_camera4.GetPosition().y, m_camera4.GetPosition().z);
		m_debugCamera.SetRotation(m_camera4.GetRotation().x, m_camera4.GetRotation().y, m_camera4.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera4.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera4.GetProjectionMatrix());
		break;

	case 5:
		// Camera 5
		m_debugCamera.SetPosition(m_camera5.GetPosition().x, m_camera5.GetPosition().y, m_camera5.GetPosition().z);
		m_debugCamera.SetRotation(m_camera5.GetRotation().x, m_camera5.GetRotation().y, m_camera5.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera5.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera5.GetProjectionMatrix());
		break;

	case 6:
		// Camera 6
		m_debugCamera.SetPosition(m_camera6.GetPosition().x, m_camera6.GetPosition().y, m_camera6.GetPosition().z);
		m_debugCamera.SetRotation(m_camera6.GetRotation().x, m_camera6.GetRotation().y, m_camera6.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera6.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera6.GetProjectionMatrix());
		break;

	case 7:
		// Camera 7
		m_debugCamera.SetPosition(m_camera7.GetPosition().x, m_camera7.GetPosition().y, m_camera7.GetPosition().z);
		m_debugCamera.SetRotation(m_camera7.GetRotation().x, m_camera7.GetRotation().y, m_camera7.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera7.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera7.GetProjectionMatrix());
		break;

	case 8:
		// Camera 8
		m_debugCamera.SetPosition(m_camera8.GetPosition().x, m_camera8.GetPosition().y, m_camera8.GetPosition().z);
		m_debugCamera.SetRotation(m_camera8.GetRotation().x, m_camera8.GetRotation().y, m_camera8.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera8.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera8.GetProjectionMatrix());
		break;

	case 9:
		// Camera 9
		m_debugCamera.SetPosition(m_camera9.GetPosition().x, m_camera9.GetPosition().y, m_camera9.GetPosition().z);
		m_debugCamera.SetRotation(m_camera9.GetRotation().x, m_camera9.GetRotation().y, m_camera9.GetRotation().z);
		XMStoreFloat4x4(&m_View, m_camera9.GetViewMatrix());
		XMStoreFloat4x4(&m_Projection, m_camera9.GetProjectionMatrix());
		break;

	default:

		break;
	}

	// Update the view and projection matrices
	m_cbData.View = XMMatrixTranspose(XMLoadFloat4x4(&m_View));
	m_cbData.Projection = XMMatrixTranspose(XMLoadFloat4x4(&m_Projection));

	// Update the rasterizer state
	if (m_fill)
	{
		m_immediateContext->RSSetState(m_fillState);
	}
	else if (!m_fill && !m_nobackfaceCulling)
	{
		m_immediateContext->RSSetState(m_wireframeState);
	}
	else if (!m_fill && m_nobackfaceCulling)
	{
		m_immediateContext->RSSetState(m_dontCullBackState);
	}

	// Write constant buffer data onto GPU
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_immediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_immediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_immediateContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_immediateContext->PSSetShader(m_pixelShader, nullptr, 0);
	m_immediateContext->PSSetSamplers(0, 1, &m_bilinearSamplerState);
	m_immediateContext->PSSetShaderResources(0, 1, &m_crateTexture);
	m_immediateContext->PSSetShaderResources(1, 1, &m_ryanlabsTexture);

	m_immediateContext->RSSetViewports(1, &m_viewport);
	// Render the scene as opaque by default
	RenderOpaque();
	m_immediateContext->OMSetDepthStencilState(nullptr, 0);
}

void DX11Framework::Update()
{
	//Static initializes this value only once
	static ULONGLONG frameStart = GetTickCount64();

	// Get the time that has passed since the last frame
	ULONGLONG frameNow = GetTickCount64();
	float deltaTime = (frameNow - frameStart) / 1000.0f;
	frameStart = frameNow;

	static float _angle = 0.0f;
	_angle += deltaTime;

	m_timeRunning += deltaTime;
	m_cbData.count = _angle;

	// Rotate the main menu object
	m_mainMenuObject->SetRotation(90, -_angle, -_angle);

	// Rotate the skybox and move it with the camera
	m_skybox->SetPosition(m_debugCamera.GetPosition().x, m_debugCamera.GetPosition().y, m_debugCamera.GetPosition().z);
	m_skybox->SetRotation(m_skybox->GetRotation().x + 0.1 * deltaTime, m_skybox->GetRotation().y + 0.1 * deltaTime,
		m_skybox->GetRotation().z + 0.1 * deltaTime);

	// Sending it to hell because it's the worst
	XMStoreFloat4x4(&m_terrain->m_matrix, XMMatrixIdentity() * XMMatrixTranslation(666, 666, 666));

	if (m_mainMenu == false)
	{
		// Stop/Start Filling the cube by pressing F3
		if (GetAsyncKeyState(VK_F3) & 0x0001)
		{
			if (m_fill)
			{
				m_fill = false;
			}
			else
			{
				m_fill = true;
				m_nobackfaceCulling = false;
			}
		}
		// Stop/Start Backface Culling by pressing F6
		if (GetAsyncKeyState(VK_F6) & 0x0001)
		{
			if (m_nobackfaceCulling)
			{
				m_nobackfaceCulling = false;
			}
			else
			{
				m_nobackfaceCulling = true;
				m_fill = false;
			}
		}

		// Stop/Start Wave Filter the cube by pressing F7
		if (GetAsyncKeyState(VK_F7) & 0x0001)
		{
			if (m_cbData.waveFilter == 1)
			{
				m_cbData.waveFilter = 0;
			}
			else if (m_cbData.waveFilter == 0)
			{
				m_cbData.waveFilter = 1;
			}
		}

		// Stop/Start Wave Filter X the cube by pressing F7
		if (GetAsyncKeyState(VK_F8) & 0x0001)
		{
			if (m_cbData.waveFilterX == 1)
			{
				m_cbData.waveFilterX = 0;
			}
			else if (m_cbData.waveFilterX == 0)
			{
				m_cbData.waveFilterX = 1;
			}
		}

		// Stop/Start Pixelate Filter the cube by pressing TAB
		if (GetAsyncKeyState(VK_TAB) & 0x0001)
		{
			if (m_cbData.pixelateFilter == 1)
			{
				m_cbData.pixelateFilter = 0;
			}
			else if (m_cbData.pixelateFilter == 0)
			{
				m_cbData.pixelateFilter = 1;
			}
		}
		// Stop/Start Gooch Shader the cube by pressing F11
		if (GetAsyncKeyState(VK_F11) & 0x0001)
		{
			if (m_cbData.goochShading == 1)
			{
				m_cbData.goochShading = 0;
				m_cbData.LightON = 1;
				m_cbData.hasTexture = 1;
			}
			else if (m_cbData.goochShading == 0)
			{
				m_cbData.goochShading = 1;
				m_cbData.LightON = 0;
				m_cbData.hasTexture = 0;
			}
		}

		// Stop/Start Light by pressing F2
		if (GetAsyncKeyState(VK_F2) & 0x0001)
		{
			if (m_cbData.LightON == 1)
			{
				m_cbData.LightON = 0;
			}
			else if (m_cbData.LightON == 0)
			{
				m_cbData.LightON = 1;
			}
		}

		// Stop/Start Texture by pressing F1
		if (GetAsyncKeyState(VK_F1) & 0x0001)
		{
			if (m_cbData.hasTexture == 1)
			{
				m_cbData.hasTexture = 0;
			}
			else
			{
				m_cbData.hasTexture = 1;
			}
		}
		///////////////////////////////////////////////////////////
		if (GetAsyncKeyState(VK_F5) & 0x0001)
		{
			if (m_textRendering)
			{
				m_textRendering = false;
			}
			else
			{
				m_textRendering = true;
			}
		}

		// Stop/Start Rotating the cube by pressing F4
		if (GetAsyncKeyState(VK_F4) & 0x0001)
		{
			if (m_rotate)
			{
				m_nobackfaceCulling = false;
				m_fill = true;
				m_rotate = false;
				m_debugCamera.SetPosition(m_startingCameraPosition.x, m_startingCameraPosition.y,
					m_startingCameraPosition.z);
				m_debugCamera.SetRotation(0, 0, 0);
			}
			else
			{
				m_nobackfaceCulling = false;
				m_fill = true;
				m_rotate = true;
				m_debugCamera.SetPosition(m_startingCameraPosition.x, m_startingCameraPosition.y,
					m_startingCameraPosition.z);
				m_debugCamera.SetRotation(0, 0, 0);
			}
		}

		if (m_rotate)
		{
			// Rotate the cubes
			XMStoreFloat4x4(
				&m_World, XMMatrixIdentity() * XMMatrixRotationY(_angle) * XMMatrixScaling(2.0f, 2.0f, 2.0f));

			XMStoreFloat4x4(
				&m_World2,
				XMMatrixIdentity() * XMMatrixRotationY(-_angle) * XMMatrixTranslation(6, 0, 2) *
				XMMatrixRotationY(-_angle));

			// Rotate some of the game objects
			for (const auto& gameObject : m_gameObjects)
			{
				if (gameObject->m_Name == "Airplane")
				{
					gameObject->SetRotation(0, -_angle, 0);
				}
				else if (gameObject->m_Name == "Car")
				{
					gameObject->SetRotation(-_angle, -_angle, 0);
				}
			}

			// I hate this
			XMStoreFloat4x4(
				&m_World3,
				XMMatrixIdentity() * XMMatrixRotationY(-_angle) * XMMatrixRotationX(-_angle) * (
					XMMatrixTranslation(6, 0, 2) *
					XMMatrixTranslation(2, 0, 0))
				* XMMatrixRotationY(-_angle) * XMMatrixScaling(0.45f, 0.45f, 0.45f) * XMLoadFloat4x4(&m_World2));

			XMStoreFloat4x4(&m_Pyramid, XMMatrixIdentity() * XMMatrixTranslation(10, 0, 2) * XMLoadFloat4x4(&m_World2));
		}
		else
		{
			XMStoreFloat4x4(&m_World, XMMatrixIdentity() * XMMatrixScaling(2.0f, 2.0f, 2.0f));

			XMStoreFloat4x4(
				&m_World2,
				XMMatrixIdentity() * XMMatrixTranslation(6, 0, 2));

			// Rotate some of the game objects
			for (const auto& gameObject : m_gameObjects)
			{
				if (gameObject->m_Name == "Airplane")
				{
					gameObject->SetRotation(0, 0, 0);
				}

				else if (gameObject->m_Name == "Car")
				{
					gameObject->SetRotation(-_angle, -_angle, 0);
				}
			}

			XMStoreFloat4x4(
				&m_World3,
				XMMatrixIdentity() * (XMMatrixTranslation(6, 0, 2) *
					XMMatrixTranslation(2, 0, 0))
				* XMMatrixScaling(0.45f, 0.45f, 0.45f) * XMLoadFloat4x4(&m_World2));

			XMStoreFloat4x4(&m_Pyramid, XMMatrixIdentity() * XMMatrixTranslation(10, 0, 2) * XMLoadFloat4x4(&m_World2));
		}

		// Reset the filters and go back to the main menu
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
		{
			m_mainMenu = true;
			m_debugCamera.SetPosition(m_startingCameraPosition.x, m_startingCameraPosition.y,
				m_startingCameraPosition.z);
			m_debugCamera.SetRotation(0, 0, 0);
			m_cbData.pixelateFilter = 0;
			m_cbData.goochShading = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.waveFilter = 0;
			m_nobackfaceCulling = false;
			m_fill = true;
			m_cameraNumActive = 0;
			m_cbData.LightON = 1;
			m_cbData.hasTexture = 1;
			m_rotate = false;
			m_fill = true;
			m_textRendering = true;
		}

		// Change the active camera by pressing the number keys
		if (GetAsyncKeyState(VK_NUMPAD0) & 0xFFFF)
		{
			m_cameraNumActive = 0;
		}
		else if (GetAsyncKeyState(VK_NUMPAD1) & 0xFFFF)
		{
			m_cameraNumActive = 1;
			m_nobackfaceCulling = false;
			m_fill = true;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
		}
		else if (GetAsyncKeyState(VK_NUMPAD3) & 0xFFFF)
		{
			m_cameraNumActive = 2;
			m_nobackfaceCulling = true;
			m_fill = false;
			m_cbData.waveFilter = 1;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
		}
		else if (GetAsyncKeyState(VK_NUMPAD2) & 0xFFFF)
		{
			m_cameraNumActive = 3;
			m_cbData.waveFilterX = 1;
			m_cbData.waveFilter = 0;
			m_nobackfaceCulling = false;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
			m_fill = true;
		}
		else if (GetAsyncKeyState(VK_NUMPAD4) & 0xFFFF)
		{
			m_cameraNumActive = 4;
			m_nobackfaceCulling = true;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 1;
			m_cbData.pixelateFilter = 0;
			m_fill = false;
		}
		else if (GetAsyncKeyState(VK_NUMPAD5) & 0xFFFF)
		{
			m_cameraNumActive = 5;
			m_nobackfaceCulling = false;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
			m_fill = true;
		}
		else if (GetAsyncKeyState(VK_NUMPAD6) & 0xFFFF)
		{
			m_cameraNumActive = 6;
			m_nobackfaceCulling = true;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
			m_fill = false;
		}
		else if (GetAsyncKeyState(VK_NUMPAD7) & 0xFFFF)
		{
			m_cameraNumActive = 7;
			m_nobackfaceCulling = false;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
			m_fill = true;
		}
		else if (GetAsyncKeyState(VK_NUMPAD8) & 0xFFFF)
		{
			m_cameraNumActive = 8;
			m_nobackfaceCulling = false;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 1;
			m_fill = true;
		}
		else if (GetAsyncKeyState(VK_NUMPAD9) & 0xFFFF)
		{
			m_cameraNumActive = 9;
			m_nobackfaceCulling = false;
			m_cbData.waveFilter = 0;
			m_cbData.waveFilterX = 0;
			m_cbData.goochShading = 0;
			m_cbData.pixelateFilter = 0;
			m_fill = false;
		}

		if (m_cameraNumActive == 0)
		{
			// I should move this to the camera class, but it was a little tricky parsing UI side of things,
			// so for now, I'm just going to leave it here.
			HandleDebugMovement(deltaTime);
		}
	}
	else
	{
		// Main Menu Control Logic
		if (GetAsyncKeyState(VK_LEFT) & 0x0001)
		{
			if (m_start == false && m_playVideo == false)
			{
				m_start = true;
			}
			if (m_start == false && m_playVideo)
			{
				m_playVideo = false;
			}
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x0001)
		{
			if (m_start == false && m_playVideo == false)
			{
				m_playVideo = true;
			}
			if (m_start == true && m_playVideo == false)
			{
				m_LEFTState = Key_DOWN;
				m_start = false;
			}
		}
		if (GetAsyncKeyState(VK_RETURN) & 0x0001)
		{
			if (m_start)
			{
				m_cbData.waveFilter = 0;
				m_cbData.LightON = 1;
				m_cbData.hasTexture = 1;
				m_rotate = false;
				m_fill = true;
				m_textRendering = true;
				m_mainMenu = false;
			}
			else if (m_playVideo)
			{
				// Play the tutorial video
				system("start TutorialVideo.mp4");
			}
			else if (m_start == false)
			{
				PostQuitMessage(0);
			}
		}

		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
		{
			PostQuitMessage(0);
		}
	}
}
#pragma endregion

#pragma region Destructor
DX11Framework::~DX11Framework()
{
	// Check if the objects are not null before releasing them
	if (m_immediateContext)
	{
		m_immediateContext->Release();
		m_immediateContext = nullptr;
	}
	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
	if (m_dxgiDevice)
	{
		m_dxgiDevice->Release();
		m_dxgiDevice = nullptr;
	}
	if (m_dxgiFactory)
	{
		m_dxgiFactory->Release();
		m_dxgiFactory = nullptr;
	}
	if (m_frameBufferView)
	{
		m_frameBufferView->Release();
		m_frameBufferView = nullptr;
	}
	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}
	if (m_fillState)
	{
		m_fillState->Release();
		m_fillState = nullptr;
	}
	if (m_wireframeState)
	{
		m_wireframeState->Release();
		m_wireframeState = nullptr;
	}
	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}
	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
	if (m_vertexShaderSkybox)
	{
		m_vertexShaderSkybox->Release();
		m_vertexShaderSkybox = nullptr;
	}
	if (m_vertexShaderHeightmap)
	{
		m_vertexShaderHeightmap->Release();
		m_vertexShaderHeightmap = nullptr;
	}
	if (m_depthStencilSkybox)
	{
		m_depthStencilSkybox->Release();
		m_depthStencilSkybox = nullptr;
	}
	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}
	if (m_terrainInputLayout)
	{
		m_terrainInputLayout->Release();
		m_terrainInputLayout = nullptr;
	}
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
	if (m_pixelShaderSkybox)
	{
		m_pixelShaderSkybox->Release();
		m_pixelShaderSkybox = nullptr;
	}
	if (m_pixelShaderHeightmap)
	{
		m_pixelShaderHeightmap->Release();
		m_pixelShaderHeightmap = nullptr;
	}
	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = nullptr;
	}
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
	if (m_bilinearSamplerState)
	{
		m_bilinearSamplerState->Release();
		m_bilinearSamplerState = nullptr;
	}
	if (m_crateTexture)
	{
		m_crateTexture->Release();
		m_crateTexture = nullptr;
	}
	if (m_ryanlabsTexture)
	{
		m_ryanlabsTexture->Release();
		m_ryanlabsTexture = nullptr;
	}
	if (m_skyboxTexture)
	{
		m_skyboxTexture->Release();
		m_skyboxTexture = nullptr;
	}
	if (m_wTexture)
	{
		m_wTexture->Release();
		m_wTexture = nullptr;
	}
	if (m_aTexture)
	{
		m_aTexture->Release();
		m_aTexture = nullptr;
	}
	if (m_sTexture)
	{
		m_sTexture->Release();
		m_sTexture = nullptr;
	}
	if (m_dTexture)
	{
		m_dTexture->Release();
		m_dTexture = nullptr;
	}
	if (m_qTexture)
	{
		m_qTexture->Release();
		m_qTexture = nullptr;
	}
	if (m_eTexture)
	{
		m_eTexture->Release();
		m_eTexture = nullptr;
	}
	if (m_upTexture)
	{
		m_upTexture->Release();
		m_upTexture = nullptr;
	}
	if (m_leftTexture)
	{
		m_leftTexture->Release();
		m_leftTexture = nullptr;
	}
	if (m_downTexture)
	{
		m_downTexture->Release();
		m_downTexture = nullptr;
	}
	if (m_rightTexture)
	{
		m_rightTexture->Release();
		m_rightTexture = nullptr;
	}
	if (m_spriteBatch)
	{
		m_spriteBatch.reset();
		m_spriteBatch = nullptr;
	}
	if (m_spriteFont)
	{
		m_spriteFont.reset();
		m_spriteFont = nullptr;
	}
	if (m_pyramidVertexBuffer)
	{
		m_pyramidVertexBuffer->Release();
		m_pyramidVertexBuffer = nullptr;
	}
	if (m_pyramidIndexBuffer)
	{
		m_pyramidIndexBuffer->Release();
		m_pyramidIndexBuffer = nullptr;
	}
	vsBlob->Release();
	vsBlob = nullptr;
	psBlob->Release();
	psBlob = nullptr;
	delete m_terrain;
	m_terrain = nullptr;
	delete m_mainMenuObject;
	m_mainMenuObject = nullptr;
	delete m_skybox;
	m_skybox = nullptr;

	// Delete the game objects
	for (auto& gameObject : m_gameObjects)
	{
		if (gameObject != nullptr)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}
}
#pragma endregion