#include "RpgGameApp.h"
#include "core/RpgCommandLine.h"
#include "asset/RpgAssetSystem.h"
#include "input/RpgInputSystem.h"
#include "render/RpgRenderer.h"



RPG_LOG_DEFINE_CATEGORY(RpgLogGame, VERBOSITY_DEBUG)


RpgGameApp* g_GameApp = nullptr;


RpgGameApp::RpgGameApp() noexcept
{
	WindowState = RpgPlatformWindowSizeState::DEFAULT;

	// fps info
	FpsLimit = 60;
	FpsSampleTimer = 0.0f;
	FpsSampleFrameCount = 0;
	FpsTimeMs = 0.0f;
	FpsCountMs = 0.0f;

	g_ConsoleSystem->RegisterObjectCommandListener(this, &RpgGameApp::HandleConsoleCommand);
}


RpgGameApp::~RpgGameApp() noexcept
{
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
}


void RpgGameApp::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
}


void RpgGameApp::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
}


void RpgGameApp::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{

}


void RpgGameApp::CharInput(char c) noexcept
{
	// Ignore toggle console
	if (c == '`')
	{
		return;
	}

}


void RpgGameApp::FrameTick(int frameIndex, float deltaTime) noexcept
{
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


	// Asset loding update
	g_AssetSystem->Update();

	const RpgRectFloat windowClipRect(0.0f, 0.0f, static_cast<float>(WindowDimension.X), static_cast<float>(WindowDimension.Y));


	// GUI
	{
		
	}


	// Tick update
	{

	}


	// Post tick update
	{

	}


	g_InputSystem->Flush();
}


void RpgGameApp::FramePreRender(int frameIndex, float deltaTime, RpgRenderer& renderer) noexcept
{
	MainViewport.World = &MainWorld;
	MainViewport.SetFrameViewRotationAndPosition(frameIndex, RpgQuaternion::FromPitchYawRollDegree(60.0f, 0.0, 0.0f), RpgVector3::ZERO);

	renderer.AddSceneViewport(&MainViewport);

	// Setup renderer final texture
	//renderer.SetFinalTexture(MainViewport.GetFrameTextureRenderTarget(frameIndex).CastStatic<RpgTexture2D>());

	/*

	if (LoadingLevel)
	{
		// Normally this is done in render-world-subsystem when camera-component referencing the main viewport
		// since the level is loading and no camera gameobject yet, we call this manually
		MainRenderer->AddWorldSceneViewport(frameIndex, MainWorld, &SceneViewport);
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
	*/
}
