// Include{s}
#include "DX11Framework.h"

// Dependencies: user32.lib; d3d11.lib; d3dcompiler.lib; dxgi.lib;

#pragma region Main Entry Point

// Main entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#pragma region Application Initialization
	// Create the application
	auto application = std::make_unique<DX11Framework>();

	if (FAILED(application->Initialise(hInstance, nCmdShow)))
	{
		return -1;
	}
#pragma endregion

#pragma region Main Message Loop
	// Main message loop
	MSG msg = { nullptr };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			application->Update();
			application->Draw();
		}
	}
#pragma endregion

	return static_cast<int>(msg.wParam);
}

#pragma endregion