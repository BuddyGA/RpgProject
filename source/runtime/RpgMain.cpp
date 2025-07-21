#include "core/RpgCommandLine.h"
#include "core/RpgConsoleSystem.h"
#include "core/RpgFilePath.h"
#include "core/RpgThreadPool.h"
#include "core/RpgTimer.h"
#include "core/RpgD3D12.h"
#include "input/RpgInputWindows.h"
#include "render/RpgTexture.h"
#include "render/RpgFont.h"
#include "render/RpgMaterial.h"
#include "asset/RpgAssetImporter.h"
#include "shader/RpgShaderManager.h"
#include "render/RpgRenderPipeline.h"
#include "engine/RpgEngine.h"
#include <windowsx.h>
#include <hidusage.h>


#ifdef RPG_BUILD_DEBUG
#include "../test/core/RpgTestCore.h"
#endif // RPG_BUILD_DEBUG



constexpr const char* RPG_WINDOW_CLASS_NAME = "RpgWindow";
static RpgPointInt MousePrevPosition;


static LRESULT CALLBACK RpgMainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
		{
			g_Engine->RequestExit(true);
			return 0;
		}

		case WM_PAINT:
		{
			ValidateRect(hwnd, nullptr);
			return 0;
		}

		case WM_ERASEBKGND:
		{
			return 1;
		}

		case WM_SIZE:
		{
			RpgPlatformWindowEvent e{};

			if (wParam == SIZE_MINIMIZED)
			{
				e.State = RpgPlatformWindowSizeState::MINIMIZED;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				e.State = RpgPlatformWindowSizeState::MAXIMIZED;
			}
			else if (wParam == SIZE_RESTORED)
			{
				e.State = RpgPlatformWindowSizeState::DEFAULT;
			}

			e.Size = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			RPG_Log(RpgLogSystem, "Window size changed: %u, %u", e.Size.X, e.Size.Y);

			g_Engine->WindowSizeChanged(e);

			return 0;
		}


		case WM_MOUSEMOVE:
		{
			RpgPlatformMouseMoveEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.DeltaPosition = e.Position - MousePrevPosition;

			//RPG_LogDebug(RpgLogTemp, "MouseDelta: %i, %i", e.DeltaPosition.X, e.DeltaPosition.Y);

			g_Engine->MouseMove(e);

			MousePrevPosition = e.Position;

			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			RpgPlatformMouseWheelEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.ScrollValue.Y = GET_WHEEL_DELTA_WPARAM(wParam) / 120;

			g_Engine->MouseWheel(e);

			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_LEFT;
			e.bIsDown = true;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_LBUTTONUP:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_LEFT;
			e.bIsDown = false;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_LBUTTONDBLCLK:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = static_cast<uint8_t>(RpgInputKey::MOUSE_LEFT);
			e.bIsDoubleClick = true;

			g_Engine->MouseButton(e);

			return 0;
		}


		case WM_MBUTTONDOWN:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_MIDDLE;
			e.bIsDown = true;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_MBUTTONUP:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_MIDDLE;
			e.bIsDown = false;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_MBUTTONDBLCLK:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = static_cast<uint8_t>(RpgInputKey::MOUSE_MIDDLE);
			e.bIsDoubleClick = true;

			g_Engine->MouseButton(e);

			return 0;
		}


		case WM_RBUTTONDOWN:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_RIGHT;
			e.bIsDown = true;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_RBUTTONUP:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = RpgInputKey::MOUSE_RIGHT;
			e.bIsDown = false;

			g_Engine->MouseButton(e);

			return 0;
		}

		case WM_RBUTTONDBLCLK:
		{
			RpgPlatformMouseButtonEvent e{};
			e.Position = RpgPointInt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			e.Button = static_cast<uint8_t>(RpgInputKey::MOUSE_RIGHT);
			e.bIsDoubleClick = true;

			g_Engine->MouseButton(e);

			return 0;
		}


		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			RpgPlatformKeyboardEvent e{};
			e.Button = RpgInputWindows::MapWindowsInputToKeyButton(wParam, lParam);
			e.bIsDown = true;

			g_Engine->KeyboardButton(e);

			return 0;
		}


		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			RpgPlatformKeyboardEvent e{};
			e.Button = RpgInputWindows::MapWindowsInputToKeyButton(wParam, lParam);
			e.bIsDown = false;

			g_Engine->KeyboardButton(e);

			return 0;
		}


		case WM_CHAR:
		{
			g_Engine->CharInput(static_cast<char>(wParam));
			return 0;
		}


		default:
			break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}



int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);


// ------------------------------------------------------------------------------------------------- //
// 	Initialization
// ------------------------------------------------------------------------------------------------- //

	RpgCommandLine::Initialize(lpCmdLine);


#ifdef RPG_BUILD_DEBUG
	RpgPlatformLog::Initialize(RpgPlatformLog::VERBOSITY_DEBUG, "RpgGame.log");
#else
	RpgPlatformLog::Initialize(RpgPlatformLog::VERBOSITY_LOG, "RpgGame.log");
#endif // RPG_BUILD_DEBUG


	g_ConsoleSystem = new RpgConsoleSystem();


	RpgPlatformProcess::Initialize();
	RpgFileSystem::Initialize();


#ifdef RPG_BUILD_DEBUG
	// Run tests
	{
		RpgTest::Core::Execute();
	}
#endif // RPG_BUILD_DEBUG


	// TODO: Read config from <RpgGame.config>

	// TODO: Steam init

	RpgThreadPool::Initialize(1);

	RpgD3D12::Initialize();
	RpgShaderManager::Initialize();
	RpgRenderPipeline::Initialize();

	RpgTexture2D::s_CreateDefaults();
	RpgFont::s_CreateDefaults();
	RpgMaterial::s_CreateDefaults();

	// Compile default materials
	{
		RpgSharedMaterial defaultMaterials[RpgMaterialDefault::MAX_COUNT];
		for (int m = 0; m < RpgMaterialDefault::MAX_COUNT; ++m)
		{
			defaultMaterials[m] = RpgMaterial::s_GetDefault(static_cast<RpgMaterialDefault::EType>(m));
		}

		RpgRenderPipeline::AddMaterials(defaultMaterials, RpgMaterialDefault::MAX_COUNT);
		RpgRenderPipeline::CompileMaterialPSOs(true);
	}


	// Initialize asset manager
	

	// Initialize asset importer
#ifndef RPG_BUILD_SHIPPING
	g_AssetImporter = new RpgAssetImporter();
#endif // !RPG_BUILD_SHIPPING


	// Initialize engine
	g_Engine = new RpgEngine();

	// main window
	{
		WNDCLASSEXA windowClass{};
		windowClass.cbSize = sizeof(WNDCLASSEXA);
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.lpszClassName = RPG_WINDOW_CLASS_NAME;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hbrBackground = NULL;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		windowClass.lpfnWndProc = RpgMainWndProc;
		windowClass.lpszMenuName = NULL;
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		RegisterClassExA(&windowClass);


		RpgPointInt windowResolution(1600, 900);

		const int cmdArgResX = RpgCommandLine::GetCommandValueInt("resx");
		if (cmdArgResX > 0)
		{
			windowResolution.X = cmdArgResX;
		}

		const int cmdArgResY = RpgCommandLine::GetCommandValueInt("resy");
		if (cmdArgResY > 0)
		{
			windowResolution.Y = cmdArgResY;
		}

		RPG_Log(RpgLogSystem, "Create game window (%i, %i)", windowResolution.X, windowResolution.Y);

		RECT rect{};
		rect.left = 0;
		rect.top = 0;
		rect.right = windowResolution.X;
		rect.bottom = windowResolution.Y;

		AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

		HWND mainWindowHandle = CreateWindowExA(0, RPG_WINDOW_CLASS_NAME, "RpgGame_v0.0_alpha", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (rect.right - rect.left), (rect.bottom - rect.top), NULL, NULL, GetModuleHandle(NULL), NULL);

		if (mainWindowHandle)
		{
			ShowWindow(mainWindowHandle, SW_SHOW);
		}
		else
		{
			MessageBoxA(NULL, "Fail create application window!", "Error", MB_ICONERROR | MB_OK);
			TerminateProcess(GetCurrentProcess(), 1);
		}

		RpgPlatformProcess::SetMainWindowHandle(mainWindowHandle);
	}

	g_Engine->Initialize();


// ------------------------------------------------------------------------------------------------- //
// 	Main Loop
// ------------------------------------------------------------------------------------------------- //
	uint64_t FrameCounter = 0;
	bool bRunning = true;

	RpgTimer Timer;
	Timer.Start();


	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER fpsFrameStart, fpsFrameEnd;
	
	MSG msg{};

	while (bRunning)
	{
		QueryPerformanceCounter(&fpsFrameStart);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bRunning = false;
			}
		}


		if (!bRunning)
		{
			break;
		}


		Timer.Tick();
		g_Engine->FrameTick(FrameCounter, Timer.GetDeltaTimeSeconds());

		const int fpsLimit = g_Engine->FpsLimit;

		if (g_Engine->IsWindowMinimized())
		{
			Sleep(30);
		}
		else if (fpsLimit > 0)
		{
			const float targetFrameMs = 1000.0f / fpsLimit;
			float remainingMs = 0.0f;

			do
			{
				QueryPerformanceCounter(&fpsFrameEnd);
				const float durationMs = (fpsFrameEnd.QuadPart - fpsFrameStart.QuadPart) * 1000.0f / frequency.QuadPart;
				remainingMs = targetFrameMs - durationMs;

				if (remainingMs > 10.0f)
				{
					Sleep(2);
				}
			}
			while (remainingMs > 0.000333f);
		}

		++FrameCounter;
	}

	RPG_Log(RpgLogSystem, "Exit game");


// ------------------------------------------------------------------------------------------------- //
// 	Shutdown
// ------------------------------------------------------------------------------------------------- //
	delete g_Engine;

	RpgMaterial::s_DestroyDefaults();
	RpgFont::s_DestroyDefaults();
	RpgTexture2D::s_DestroyDefaults();

	RpgShaderManager::Shutdown();
	RpgRenderPipeline::Shutdown();
	RpgD3D12::Shutdown();

	RpgThreadPool::Shutdown();
	RpgPlatformProcess::Shutdown();

	delete g_ConsoleSystem;

	return 0;
}
