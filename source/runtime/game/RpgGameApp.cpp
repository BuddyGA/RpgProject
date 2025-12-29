#include "RpgGameApp.h"
#include "core/RpgCommandLine.h"
#include "core/asset/RpgAssetSystem.h"
#include "core/input/RpgInputSystem.h"
#include "render/RpgRenderThread.h"
#include "render/world/RpgRenderComponent.h"


#ifndef RPG_BUILD_SHIPPING
#include "RpgEditor.h"
#include "../../test/gui/RpgTestGui.h"
#include "../../test/game/RpgTestGame.h"
#endif // !RPG_BUILD_SHIPPING



RPG_LOG_DEFINE_CATEGORY(RpgLogGame, VERBOSITY_DEBUG)


RpgGameApp* g_GameApp = nullptr;


RpgGameApp::RpgGameApp() noexcept
{
	WindowState = RpgPlatformWindowSizeState::DEFAULT;

	SceneViewport.bIsMainViewport = true;
	LoadingLevel = nullptr;
	CurrentState = RpgGameState::INTRO;

	// fps info
	FpsLimit = 60;
	FpsSampleTimer = 0.0f;
	FpsSampleFrameCount = 0;
	FpsTimeMs = 0.0f;
	FpsCountMs = 0.0f;

	RpgRenderThread::Initialize();
}


RpgGameApp::~RpgGameApp() noexcept
{
#ifndef RPG_BUILD_SHIPPING
	delete g_Editor;
#endif // !RPG_BUILD_SHIPPING

	RpgRenderThread::Shutdown();
}


void RpgGameApp::Initialize() noexcept
{
	g_ConsoleSystem->RegisterObjectCommandListener(this, &RpgGameApp::HandleConsoleCommand);

	// main world
	MainWorld = CreateWorld<RpgGameWorld>("world_game");

	// main renderer
	MainRenderer = RpgPointer::MakeUnique<RpgRenderer>(RpgPlatformProcess::GetMainWindowHandle(), !RpgCommandLine::HasCommand("novsync"));

	// gui canvas
	GuiCanvas.Name = "engine_canvas";

	// gui console
	GuiConsole = GuiCanvas.AddChild<RpgGuiConsole>();


#ifndef RPG_BUILD_SHIPPING
	g_Editor = new RpgEditor();
	g_Editor->SetupGUI(GuiCanvas);

	// test gui
	//RpgTest::Gui::Create(GuiCanvas);

	// test level
	RpgTest::Game::Create(MainWorld);

	g_Editor->LevelLoaded(MainWorld);
#endif // !RPG_BUILD_SHIPPING

}


void RpgGameApp::RequestExit(bool bAskConfirmation) noexcept
{
	if (!bAskConfirmation)
	{
		PostQuitMessage(0);
		return;
	}

	if (MessageBoxA(RpgPlatformProcess::GetMainWindowHandle(), "Are you sure you want to exit?", "Confirmation", MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		PostQuitMessage(0);
	}
}


void RpgGameApp::HandleConsoleCommand(const RpgName& command, const RpgConsoleCommandParams& params) noexcept
{
	if (command == "exit")
	{
		RequestExit(false);
	}
}


void RpgGameApp::WindowSizeChanged(const RpgPlatformWindowEvent& e) noexcept
{
	WindowDimension = e.Size;
	WindowState = e.State;
}


void RpgGameApp::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	GuiContext.MouseMove(e);
}


void RpgGameApp::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	GuiContext.MouseWheel(e);
}


void RpgGameApp::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	GuiContext.MouseButton(e);
}


void RpgGameApp::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	GuiContext.KeyboardButton(e);

	if (e.bIsDown)
	{
		if (e.Button == RpgInputKey::KEYBOARD_TILDE)
		{
			GuiConsole->Toggle();
		}
	}

#ifndef RPG_BUILD_SHIPPING
	g_Editor->KeyboardButton(e);
#endif // !RPG_BUILD_SHIPPING

}


void RpgGameApp::CharInput(char c) noexcept
{
	// Ignore toggle console
	if (c == '`')
	{
		return;
	}

	GuiContext.CharInput(c);
}


void RpgGameApp::FrameTick(uint64_t frameCounter, float deltaTime) noexcept
{
	const int frameIndex = frameCounter % RPG_FRAME_BUFFERING;

	// Calculate average FPS
	{
		const int FPS_SAMPLE_COUNT = 3;

		if (FpsSampleFrameCount == FPS_SAMPLE_COUNT)
		{
			FpsTimeMs = (FpsSampleTimer * 1000.0f) / FPS_SAMPLE_COUNT;
			FpsCountMs = static_cast<float>(FPS_SAMPLE_COUNT) / FpsSampleTimer;
			FpsSampleTimer = 0.0f;
			FpsSampleFrameCount = 0;
			FpsString = RpgString::Format("%.2f ms (%.0f FPS)", FpsTimeMs, FpsCountMs);
		}

		FpsSampleTimer += deltaTime;
		++FpsSampleFrameCount;
	}


	// Begin frame
	{
		g_AssetSystem->Update();

		MainWorld->BeginFrame(frameIndex);

		if (LoadingLevel)
		{
			RPG_LogDebug(RpgLogGame, "Loading progress: %.2f", LoadingLevel->GetLoadingProgress());

			if (LoadingLevel->IsLoaded())
			{
			#ifndef RPG_BUILD_SHIPPING
				g_Editor->LevelLoaded(MainWorld);
			#endif // !RPG_BUILD_SHIPPING

				LoadingLevel = nullptr;
			}

			//SceneViewport.GetFrameMeshes(frameIndex).Clear();
			//SceneViewport.GetFrameLights(frameIndex).Clear();
		}
		else
		{
			RpgRenderComponent_Camera* mainCameraComp = !MainCameraObject.IsNull() ? MainCameraObject.GetComponent<RpgRenderComponent_Camera>() : nullptr;

			if (mainCameraComp && WindowState != RpgPlatformWindowSizeState::MINIMIZED)
			{
				mainCameraComp->RenderTargetDimension = WindowDimension;
			}
		}
	}

	const RpgRectFloat windowClipRect(0.0f, 0.0f, static_cast<float>(WindowDimension.X), static_cast<float>(WindowDimension.Y));

	// GUI
	{
		GuiContext.Begin();

		if (WindowState != RpgPlatformWindowSizeState::MINIMIZED)
		{
			GuiCanvas.UpdateWidgets(GuiContext, windowClipRect);
		}

		GuiContext.End();
	}


	// Tick update
	{
		MainWorld->DispatchTickUpdate(deltaTime);
	}


	// Post tick update
	{
		MainWorld->DispatchPostTickUpdate();
	}


	// Render
	RpgRenderThread::WaitFrame(frameIndex);
	{
		RpgD3D12::BeginFrame(frameIndex);

		MainRenderer->BeginRender(frameIndex, deltaTime);
		{
			MainRenderer->RegisterWorld(MainWorld);

			if (!LoadingLevel)
			{
				// Setup renderer final texture
				MainRenderer->SetFinalTexture(frameIndex, SceneViewport.GetTextureRenderTarget(frameIndex).CastStatic<RpgTexture2D>());
			}

			// Dispatch render
			MainWorld->DispatchRender(frameIndex, MainRenderer.Get());

			// Render 2D
			RpgRenderer2D& renderer2d = MainRenderer->GetRenderer2D();

		#ifndef RPG_BUILD_SHIPPING
			g_Editor->Render2d(renderer2d);
		#endif // !RPG_BUILD_SHIPPING

			// Fps info
			{
				RpgColor fpsTextColor;

				if (FpsCountMs < 30)
				{
					fpsTextColor = RpgColor::RED;
				}
				else if (FpsCountMs < 50)
				{
					fpsTextColor = RpgColor::YELLOW;
				}
				else
				{
					fpsTextColor = RpgColor::GREEN;
				}

				const RpgPointFloat fpsTextPos(static_cast<float>(renderer2d.GetViewportDimension().X) - 110.0f, 8.0f);
				renderer2d.AddText(*FpsString, FpsString.GetLength(), fpsTextPos, fpsTextColor);
			}

			// Loading info
			if (LoadingLevel)
			{
				static RpgString loadingString = RpgString::Format("Loading: %.2f", LoadingLevel->GetLoadingProgress());
				const RpgPointFloat loadingTextPos(16.0f, static_cast<float>(renderer2d.GetViewportDimension().Y) - 160.0f);
				renderer2d.AddText(*loadingString, loadingString.GetLength(), loadingTextPos, RpgColor::WHITE);
			}

			// GUI
			GuiCanvas.Render(GuiContext, renderer2d, 255, windowClipRect);
		}
		MainRenderer->EndRender(frameIndex, deltaTime);
	}
	RpgRenderThread::ExecuteFrame(frameCounter, frameIndex, deltaTime, MainRenderer.Get());


	// End frame
	MainWorld->EndFrame(frameIndex);

	g_InputSystem->Flush();
}


void RpgGameApp::OpenLevel(const RpgStringID& levelAssetPath) noexcept
{
	if (levelAssetPath.IsEmpty())
	{
		return;
	}

	RPG_CONSOLE_Log(RpgLogGame, "Open level (%s)", *levelAssetPath.ToString());

	// TODO: Show loading screen
	MainCameraObject = RpgGameObject();
	MainWorld->ClearLevels();
	LoadingLevel = MainWorld->LoadLevelAsync(levelAssetPath);
}


void RpgGameApp::SetMainCamera(RpgGameObject cameraObject) noexcept
{
	MainCameraObject = cameraObject;

	if (MainCameraObject.IsNull())
	{
		return;
	}

	RpgRenderComponent_Camera* cameraComp = MainCameraObject.GetComponent<RpgRenderComponent_Camera>();
	RPG_Check(cameraComp);
	cameraComp->Viewport = &SceneViewport;
	cameraComp->bActivated = true;
}
