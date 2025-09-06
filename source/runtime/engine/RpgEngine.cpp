#include "RpgEngine.h"
#include "core/RpgCommandLine.h"
#include "core/RpgAssetSystem.h"
#include "core/RpgInputSystem.h"
#include "physics/world/RpgPhysicsComponent.h"
#include "physics/world/RpgPhysicsWorldSubsystem.h"
#include "render/RpgRenderThread.h"
#include "render/world/RpgRenderComponent.h"
#include "render/world/RpgRenderWorldSubsystem.h"
#include "animation/world/RpgAnimationComponent.h"
#include "animation/world/RpgAnimationWorldSubsystem.h"


#ifndef RPG_BUILD_SHIPPING
#include "RpgEditor.h"
#include "../../test/gui/RpgTestGui.h"
#include "../../test/engine/RpgTestEngine.h"
#endif // !RPG_BUILD_SHIPPING



RPG_LOG_DEFINE_CATEGORY(RpgLogEngine, VERBOSITY_DEBUG)


RpgEngine* g_Engine = nullptr;


RpgEngine::RpgEngine() noexcept
{
	WindowState = RpgPlatformWindowSizeState::DEFAULT;

	// fps info
	FpsLimit = 60;
	FpsSampleTimer = 0.0f;
	FpsSampleFrameCount = 0;
	FpsTimeMs = 0.0f;
	FpsCountMs = 0.0f;

	RpgRenderThread::Initialize();
}


RpgEngine::~RpgEngine() noexcept
{
	RpgRenderThread::Shutdown();
}


void RpgEngine::Initialize() noexcept
{
	g_ConsoleSystem->RegisterObjectCommandListener(this, &RpgEngine::HandleConsoleCommand);


#ifndef RPG_BUILD_SHIPPING

	// add engine assets
	RpgSharedTexture2D texDefWhite = RpgTexture2D::s_GetDefault_White();
	g_AssetSystem->SaveAsset<RpgTexture2D>(texDefWhite, "engine/texture");

	for (int i = 0; i < RpgMaterialDefault::MAX_COUNT; ++i)
	{
		RpgSharedMaterial matDef = RpgMaterial::s_GetDefault(static_cast<RpgMaterialDefault::EType>(i));
		g_AssetSystem->SaveAsset<RpgMaterial>(matDef, "engine/material");
	}

	// scanning asset files
	g_AssetSystem->ScanAssetFiles();

#endif // !RPG_BUILD_SHIPPING


	// main world
	{
		MainWorld = CreateWorld("world_main");

		// Subsystems
		MainWorld->Subsystem_Register<RpgPhysicsWorldSubsystem>();
		MainWorld->Subsystem_Register<RpgAnimationWorldSubsystem>();
		MainWorld->Subsystem_Register<RpgRenderWorldSubsystem>();

		// Components
		MainWorld->Component_Register<RpgPhysicsComponent_Filter>();
		MainWorld->Component_Register<RpgPhysicsComponent_Collision>();
		MainWorld->Component_Register<RpgRenderComponent_Mesh>();
		MainWorld->Component_Register<RpgRenderComponent_Light>();
		MainWorld->Component_Register<RpgRenderComponent_Camera>();
		MainWorld->Component_Register<RpgAnimationComponent_AnimSkeletonPose>();
	}


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
	RpgTest::Engine::Create(MainWorld);

	g_Editor->LevelLoaded(MainWorld);
#endif // !RPG_BUILD_SHIPPING

}


void RpgEngine::HandleConsoleCommand(const RpgName& command, const RpgConsoleCommandParams& params) noexcept
{
	if (command == "exit")
	{
		RequestExit(false);
	}
}


void RpgEngine::WindowSizeChanged(const RpgPlatformWindowEvent& e) noexcept
{
	WindowDimension = e.Size;
	WindowState = e.State;
}


void RpgEngine::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	GuiContext.MouseMove(e);
}


void RpgEngine::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	GuiContext.MouseWheel(e);
}


void RpgEngine::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	GuiContext.MouseButton(e);
}


void RpgEngine::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
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


void RpgEngine::CharInput(char c) noexcept
{
	// Ignore toggle console
	if (c == '`')
	{
		return;
	}

	GuiContext.CharInput(c);
}


void RpgEngine::FrameTick(uint64_t frameCounter, float deltaTime) noexcept
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
		MainWorld->BeginFrame(frameIndex);
		g_AssetSystem->Update();
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
		RpgRenderComponent_Camera* mainCameraComp = MainCameraObject.IsValid() ? MainWorld->GameObject_GetComponent<RpgRenderComponent_Camera>(MainCameraObject) : nullptr;

		if (mainCameraComp && WindowState != RpgPlatformWindowSizeState::MINIMIZED)
		{
			mainCameraComp->RenderTargetDimension = WindowDimension;
		}

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

			// Setup renderer default final texture
			MainRenderer->SetFinalTexture(frameIndex, SceneViewport.GetTextureRenderTarget(frameIndex).CastStatic<RpgTexture2D>());

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


void RpgEngine::RequestExit(bool bAskConfirmation) noexcept
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


RpgWorld* RpgEngine::CreateWorld(const RpgName& name) noexcept
{
	const int index = Worlds.GetCount();
	Worlds.AddValue(RpgPointer::MakeUnique<RpgWorld>(name));

	return Worlds[index].Get();
}


void RpgEngine::DestroyWorld(RpgWorld*& world) noexcept
{
	RPG_Check(world);

	const int index = Worlds.FindIndexByCompare(world);
	if (index != RPG_INDEX_INVALID)
	{
		Worlds.RemoveAt(index);
		world = nullptr;

		return;
	}

	RPG_LogWarn(RpgLogEngine, "Fail to destroy world. World (%s) not found!", *world->GetName());
}


void RpgEngine::SaveLevel() noexcept
{
	
}


void RpgEngine::LoadLevel(const RpgString& levelAssetPath) noexcept
{
	RPG_NotImplementedYet();
}


void RpgEngine::SetMainCamera(RpgGameObjectID cameraObject) noexcept
{
	MainCameraObject = cameraObject;

	if (!MainWorld->GameObject_IsValid(MainCameraObject))
	{
		return;
	}

	RpgRenderComponent_Camera* cameraComp = MainWorld->GameObject_GetComponent<RpgRenderComponent_Camera>(MainCameraObject);
	RPG_Check(cameraComp);
	cameraComp->Viewport = &SceneViewport;
	cameraComp->bActivated = true;
}
