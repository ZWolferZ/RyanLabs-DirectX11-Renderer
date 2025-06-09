#pragma once

// Include{s}
#include "GameObject.h"
#include "Camera.h"
#include "Terrain.h"

class DX11Framework
{
public:
#pragma region Initialization Methods
	// Initializes the DX11 framework, including creating the window handle, D3D device, shaders, and buffers
	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	// Creates the window handle for the application
	HRESULT CreateWindowHandle(HINSTANCE hInstance, int nCmdShow);

	// Creates the D3D device and device context
	HRESULT CreateD3DDevice();

	// Creates the swap chain and frame buffer
	HRESULT CreateSwapChainAndFrameBuffer();

	// Initializes the input layout for the shaders
	HRESULT InitInputLayout();

	// Initializes Simple Shader File
	HRESULT InitSimpleShaders();

	// Initializes Skybox Shaders File
	HRESULT InitSkyBoxShaders();

	// Initializes Heightmap Shader File
	HRESULT InitHeightMapShader();

	// Initializes shaders and input layout
	HRESULT InitShaders();

	// Initializes hard coded objects such cubes and pyramid
	HRESULT InitHardCodedObjects();

	// Initializes vertex and index buffers
	HRESULT InitVertexIndexBuffers();

	// Initializes pipeline variables such as rasterizer states and blend states
	HRESULT InitPipelineVariables();

	// Initializes runtime data such as the gameobjects and lights
	HRESULT InitRunTimeData();

	// Loads UI components
	HRESULT LoadUI(HRESULT hr);

	// Loads light variables from a configuration source
	void LoadLightVariables();

	// Loads camera variables from a scene configuration source
	void LoadSceneCameraVariables();

	// Loads game object data from a scene JSON file
	void LoadGameObjectDataFromSceneJSON();

	// Loads a specific game object based on the index provided
	void LoadGameObject(int i);
#pragma endregion

#pragma region Update Methods
	// Updates the state of the DX11 framework, including game logic and rendering
	void Update();

	// Handles debug movement, allowing for camera adjustments during runtime
	void HandleDebugMovement(float deltatime);

	// Updates pipeline variables such as constant buffers
	void UpdatePipelineVariables(UINT stride, UINT offset);
#pragma endregion

#pragma region Draw Methods
	// Draws a UI key state indicator at a specific screen position
	void DrawKey(KeyState state, ID3D11ShaderResourceView* texture, XMFLOAT2 position) const;

	// Draws the entire UI
	void DrawUI();

	// Draws the main menu UI
	void DrawMainMenu() const;

	// Renders the terrain
	void RenderTerrain(UINT stride, UINT offset);

	// Renders the skybox
	void RenderSkybox(UINT stride, UINT offset);

	// Draws the entire scene
	void Draw();

	// Renders transparent objects
	void RenderTransparent() const;

	// Renders opaque objects
	void RenderOpaque() const;
#pragma endregion

#pragma region Destructor
	// Destructor to release all resources and clean up
	~DX11Framework();
#pragma endregion

private:
#pragma region Window Variables
	// Window dimensions
	int m_windowWidth = 1920;
	int m_windowHeight = 1080;
#pragma endregion

#pragma region State Flags
	// State flags for various rendering and gameplay states
	bool m_fill = true;
	bool m_rotate = false;
	bool m_textRendering = true;
	bool m_mainMenu = true;
	bool m_start = true;
	bool m_playVideo = false;
	bool m_nobackfaceCulling = false;
	bool m_mouseMode = false;
#pragma endregion

#pragma region Miscellaneous Variables
	// Miscellaneous variables for angles, time, camera and speed settings
	float m_tempAngle = 0.0f;
	float m_timeRunning = 0.0f;
	int m_cameraNumActive = 0;
	const float m_cameraSpeed = 20.0f;
	const float m_rotationSpeed = 1.0f;
	const float m_deadZone = 50.0f;
#pragma endregion

#pragma region JSON Data Variables
	// JSON objects for storing scene, light, and camera variables
	nlohmann::json m_sceneData;
	nlohmann::json m_lightVariables;
	nlohmann::json m_sceneCameraVariables;
#pragma endregion

#pragma region Light Variables
	// Variables for storing light properties
	XMFLOAT3 m_lightDir = { 0, 0, 0 };
	XMFLOAT4 m_diffuseLight = { 0, 0, 0, 0 };
	XMFLOAT4 m_diffuseMaterial = { 0, 0, 0, 0 };
	XMFLOAT4 m_ambientLight = { 0, 0, 0, 0 };
	XMFLOAT4 m_ambientMaterial = { 0, 0, 0, 0 };
	XMFLOAT4 m_specularLight = { 0, 0, 0, 0 };
	XMFLOAT4 m_specularMaterial = { 0, 0, 0, 0 };
	float m_specPower = 0;
#pragma endregion

#pragma region D3D11 Objects
	// Direct3D 11 objects for rendering
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11DeviceContext* m_immediateContext = nullptr;
	ID3D11Device* m_device = nullptr;
	IDXGIDevice* m_dxgiDevice = nullptr;
	IDXGIFactory2* m_dxgiFactory = nullptr;
	ID3D11RenderTargetView* m_frameBufferView = nullptr;
	IDXGISwapChain1* m_swapChain = nullptr;
	D3D11_VIEWPORT m_viewport = {};
	ID3D11RasterizerState* m_fillState = nullptr;
	ID3D11RasterizerState* m_wireframeState = nullptr;
	ID3D11RasterizerState* m_dontCullBackState = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11VertexShader* m_vertexShaderSkybox = nullptr;
	ID3D11VertexShader* m_vertexShaderHeightmap = nullptr;
	ID3D11DepthStencilState* m_depthStencilSkybox = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
	ID3D11InputLayout* m_terrainInputLayout = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11PixelShader* m_pixelShaderSkybox = nullptr;
	ID3D11PixelShader* m_pixelShaderHeightmap = nullptr;
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	ID3D11Buffer* m_pyramidIndexBuffer = nullptr;
	ID3D11Buffer* m_pyramidVertexBuffer = nullptr;
	ID3D11BlendState* m_transparency = nullptr;
	ID3D11SamplerState* m_bilinearSamplerState = nullptr;
	ID3D11ShaderResourceView* m_crateTexture = nullptr;
	ID3D11ShaderResourceView* m_ryanlabsTexture = nullptr;
	ID3D11ShaderResourceView* m_skyboxTexture = nullptr;
	ID3D11ShaderResourceView* m_wTexture = nullptr;
	ID3D11ShaderResourceView* m_aTexture = nullptr;
	ID3D11ShaderResourceView* m_sTexture = nullptr;
	ID3D11ShaderResourceView* m_dTexture = nullptr;
	ID3D11ShaderResourceView* m_upTexture = nullptr;
	ID3D11ShaderResourceView* m_leftTexture = nullptr;
	ID3D11ShaderResourceView* m_downTexture = nullptr;
	ID3D11ShaderResourceView* m_rightTexture = nullptr;
	ID3D11ShaderResourceView* m_qTexture = nullptr;
	ID3D11ShaderResourceView* m_eTexture = nullptr;
#pragma endregion

#pragma region Key States
	// Variables to store the state of various keys
	KeyState m_WState = Key_UP;
	KeyState m_AState = Key_UP;
	KeyState m_SState = Key_UP;
	KeyState m_DState = Key_UP;
	KeyState m_QState = Key_UP;
	KeyState m_EState = Key_UP;
	KeyState m_UPState = Key_UP;
	KeyState m_LEFTState = Key_UP;
	KeyState m_DOWNState = Key_UP;
	KeyState m_RIGHTState = Key_UP;
#pragma endregion

#pragma region Game Objects
	// Collection of game objects and individual game object pointers
	std::vector<GameObject*> m_gameObjects;
	GameObject* m_mainMenuObject = nullptr;
	GameObject* m_skybox = nullptr;
#pragma endregion

#pragma region Sprite Batch & Font
	// Objects for sprite batch and font rendering
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	std::unique_ptr<SpriteFont> m_spriteFont;
#pragma endregion

#pragma region Window Handle
	// Handle to the application window
	HWND m_windowHandle = nullptr;
#pragma endregion

#pragma region Matrices
	// Matrices for various transformations and views
	XMFLOAT4X4 m_TerrainMatirx;
	XMFLOAT4X4 m_World = {};
	XMFLOAT4X4 m_World2 = {};
	XMFLOAT4X4 m_World3 = {};
	XMFLOAT4X4 m_Pyramid = {};
	XMFLOAT4X4 m_View = {};
	XMFLOAT4X4 m_Projection = {};
#pragma endregion

#pragma region Terrain
	// Pointer to the terrain object
	Terrain* m_terrain = nullptr;
#pragma endregion

#pragma region Constant Buffer
	// Constant buffer for passing data to the shaders
	ConstantBuffer m_cbData = {};
#pragma endregion

#pragma region Cameras
	// Camera objects for various viewpoints
	Camera m_debugCamera = {};
	Camera m_camera1 = {};
	Camera m_camera2 = {};
	Camera m_camera3 = {};
	Camera m_camera4 = {};
	Camera m_camera5 = {};
	Camera m_camera6 = {};
	Camera m_camera7 = {};
	Camera m_camera8 = {};
	Camera m_camera9 = {};
	XMFLOAT3 m_startingCameraPosition = { 0, 0, 0 };
#pragma endregion

#pragma region Blend Factors
	// Blend factors for transparency
	const FLOAT m_blendfactor[4] = { 1, 1, 1, 1 };
	const FLOAT m_transparencyBlendFactor[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
#pragma endregion
};
