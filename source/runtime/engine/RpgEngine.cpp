#include "RpgEngine.h"
#include "core/RpgCommandLine.h"
#include "input/RpgInputManager.h"
#include "physics/world/RpgPhysicsComponent.h"
#include "physics/world/RpgPhysicsWorldSubsystem.h"
#include "render/RpgRenderThread.h"
#include "render/world/RpgRenderComponent.h"
#include "render/world/RpgRenderWorldSubsystem.h"
#include "animation/world/RpgAnimationComponent.h"
#include "animation/world/RpgAnimationWorldSubsystem.h"
#include "asset/RpgAssetManager.h"




#include "../../test/gui/RpgTestGui.h"


#ifndef RPG_BUILD_SHIPPING
#include "RpgEditor.h"
#endif // !RPG_BUILD_SHIPPING



RPG_LOG_DEFINE_CATEGORY(RpgLogEngine, VERBOSITY_DEBUG)



/*
#pragma push_macro("free")
#undef free

#define NK_ASSERT						RPG_Assert
#define NK_UINT_DRAW_INDEX
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_KEYSTATE_BASED_INPUT
#include "thirdparty/nuklear/nuklear.h"

#pragma pop_macro("free")

static float Rpg_NuklearTextWidth(nk_handle handle, float height, const char* text, int len)
{
	return reinterpret_cast<RpgFont*>(handle.ptr)->CalculateTextDimension(text, len).X;
}


static nk_context NkContext;
static nk_user_font NkTestFont;
*/


RpgEngine* g_Engine = nullptr;


RpgEngine::RpgEngine() noexcept
{
	WindowState = RpgPlatformWindowSizeState::DEFAULT;

	//GuiConsole = nullptr;

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
	// input manager
	g_InputManager = new RpgInputManager();

	// asset manager
	g_AssetManager = new RpgAssetManager();
	g_AssetManager->ScanAssetFiles();


	// main world
	{
		MainWorld = CreateWorld("world_main");

		// Subsystems
		MainWorld->Subsystem_Register<RpgPhysicsWorldSubsystem>(0);
		MainWorld->Subsystem_Register<RpgAnimationWorldSubsystem>(1);
		MainWorld->Subsystem_Register<RpgRenderWorldSubsystem>(2);

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


	/*
	RpgSharedFont defaultRobotoFont = RpgFont::s_GetDefault_Roboto();

	NkTestFont.userdata.ptr = defaultRobotoFont.Get();
	NkTestFont.height = defaultRobotoFont->GetPixelHeight();
	NkTestFont.width = Rpg_NuklearTextWidth;

	nk_init_fixed(&NkContext, calloc(1, RPG_MEMORY_SIZE_MiB(4)), RPG_MEMORY_SIZE_MiB(4), &NkTestFont);
	*/

#ifndef RPG_BUILD_SHIPPING
	g_Editor = new RpgEditor();
	g_Editor->SetupGUI(GuiCanvas);
#endif // !RPG_BUILD_SHIPPING


	// test gui
	RpgTest::Gui::Create(GuiCanvas);

	// test level
	CreateTestLevel();


	// create main camera object
	SetMainCamera(MainWorld->GameObject_Create("camera_main"));
	MainWorld->GameObject_AttachScript(MainCameraObject, &ScriptDebugCamera);
}


void RpgEngine::WindowSizeChanged(const RpgPlatformWindowEvent& e) noexcept
{
	WindowDimension = e.Size;
	WindowState = e.State;
}


void RpgEngine::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	GuiContext.MouseMove(e);
	g_InputManager->MouseMove(e);
}


void RpgEngine::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	GuiContext.MouseWheel(e);
	g_InputManager->MouseWheel(e);
}


void RpgEngine::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	GuiContext.MouseButton(e);
	g_InputManager->MouseButton(e);
}


void RpgEngine::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	GuiContext.KeyboardButton(e);
	g_InputManager->KeyboardButton(e);

	if (e.bIsDown)
	{
		if (e.Button == RpgInputKey::KEYBOARD_TILDE)
		{
			GuiConsole->Toggle();
		}
		else if (e.Button == RpgInputKey::KEYBOARD_PLUS)
		{
			MainRenderer->Gamma += 0.01f;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_MINUS)
		{
			MainRenderer->Gamma -= 0.01f;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_0)
		{
			RpgRenderComponent_Camera* cameraComp = MainWorld->GameObject_GetComponent<RpgRenderComponent_Camera>(MainCameraObject);
			cameraComp->bFrustumCulling = !cameraComp->bFrustumCulling;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_9)
		{
			RpgAnimationWorldSubsystem* subsystem = MainWorld->Subsystem_Get<RpgAnimationWorldSubsystem>();
			subsystem->bDebugDrawSkeletonBones = !subsystem->bDebugDrawSkeletonBones;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_8)
		{
			RpgRenderWorldSubsystem* subsystem = MainWorld->Subsystem_Get<RpgRenderWorldSubsystem>();
			subsystem->bDebugDrawMeshBound = !subsystem->bDebugDrawMeshBound;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_F9)
		{
			if (MainWorld->HasStartedPlay())
			{
				MainWorld->DispatchStopPlay();
			}
			else
			{
				MainWorld->DispatchStartPlay();
			}
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
		g_AssetManager->Update();
	}


	RpgRenderComponent_Camera* mainCameraComp = MainCameraObject.IsValid() ? MainWorld->GameObject_GetComponent<RpgRenderComponent_Camera>(MainCameraObject) : nullptr;

	if (mainCameraComp && WindowState != RpgPlatformWindowSizeState::MINIMIZED)
	{
		mainCameraComp->RenderTargetDimension = WindowDimension;
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

		/*
		nk_input_begin(&NkContext);
		{
			const RpgPoint mouseCursorPos = RpgPointInt(g_InputManager->GetMouseCursorPosition());
			nk_input_motion(&NkContext, mouseCursorPos.X, mouseCursorPos.Y);

			const bool bMouseLeftDown = g_InputManager->GetKeyButtonState(RpgInputKey::MOUSE_LEFT) == RpgInputButtonState::DOWN;
			nk_input_button(&NkContext, nk_buttons::NK_BUTTON_LEFT, mouseCursorPos.X, mouseCursorPos.Y, bMouseLeftDown);
		}
		nk_input_end(&NkContext);

		enum { EASY, HARD };
		static int op = EASY;
		static float value = 0.6f;
		static int i = 20;

		if (nk_begin(&NkContext, "Show", nk_rect(50, 50, 220, 220), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) 
		{
			// fixed widget pixel width
			nk_layout_row_static(&NkContext, 30, 80, 1);

			if (nk_button_label(&NkContext, "button")) 
			{
				// event handling
			}

			// fixed widget window ratio width
			nk_layout_row_dynamic(&NkContext, 30, 2);

			if (nk_option_label(&NkContext, "easy", op == EASY)) op = EASY;
			if (nk_option_label(&NkContext, "hard", op == HARD)) op = HARD;

			// custom widget pixel width
			nk_layout_row_begin(&NkContext, NK_STATIC, 30, 2);
			{
				nk_layout_row_push(&NkContext, 50);
				nk_label(&NkContext, "Volume:", NK_TEXT_LEFT);
				nk_layout_row_push(&NkContext, 110);
				nk_slider_float(&NkContext, 0, &value, 1.0f, 0.1f);
			}
			nk_layout_row_end(&NkContext);
		}
		nk_end(&NkContext);
		*/
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

			// Setup renderer default final texture
			MainRenderer->SetFinalTexture(frameIndex, SceneViewport.GetTextureRenderTarget(frameIndex));

			// Dispatch render
			MainWorld->DispatchRender(frameIndex, MainRenderer.Get());

			// Render 2D
			RpgRenderer2D& renderer2d = MainRenderer->GetRenderer2D();
			
		#ifndef RPG_BUILD_SHIPPING
			// Debug info
			{
				static RpgString debugInfoText;

				RpgTransform mainCameraTransform = MainCameraObject.IsValid() ? MainWorld->GameObject_GetWorldTransform(MainCameraObject) : RpgTransform();
				float pitch, yaw;
				ScriptDebugCamera.GetRotationPitchYaw(pitch, yaw);

				debugInfoText = RpgString::Format(
					"WindowSize: %i, %i\n"
					"CameraPosition: %.2f, %.2f, %.2f\n"
					"CameraPitchYaw: %.2f, %.2f\n"
					"CameraFrustumCulling: %d\n"
					"Gamma: %.2f\n"
					"VSync: %d\n"
					"\n"
					"GameObject: %i\n"
					, WindowDimension.X, WindowDimension.Y
					, mainCameraTransform.Position.X, mainCameraTransform.Position.Y, mainCameraTransform.Position.Z
					, pitch, yaw
					, mainCameraComp ? mainCameraComp->bFrustumCulling : false
					, MainRenderer->Gamma
					, MainRenderer->GetVsync()
					, MainWorld->GameObject_GetCount()
				);

				renderer2d.AddText(*debugInfoText, debugInfoText.GetLength(), RpgPointFloat(8.0f, 8.0f), RpgColor(255, 255, 255));
			}
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

			/*
			const nk_command* cmd = 0;
			for (cmd = nk__begin(&NkContext); cmd != 0; cmd = nk__next(&NkContext, cmd))
			{
				switch (cmd->type)
				{
					case NK_COMMAND_SCISSOR:
					{
						const nk_command_scissor* cmdScissor = reinterpret_cast<const nk_command_scissor*>(cmd);
						//renderer2d.PushClipRect(RpgRectInt(cmdScissor->x, cmdScissor->y, cmdScissor->x + cmdScissor->w, cmdScissor->y + cmdScissor->h));

						break;
					};

					case NK_COMMAND_LINE:
					{
						const nk_command_line* cmdLine = reinterpret_cast<const nk_command_line*>(cmd);
						
						renderer2d.AddLine(
							RpgPointFloat(cmdLine->begin.x, cmdLine->begin.y),
							RpgPointFloat(cmdLine->end.x, cmdLine->end.y),
							RpgColor(cmdLine->color.r, cmdLine->color.g, cmdLine->color.b, cmdLine->color.a)
						);

						break;
					};

					case NK_COMMAND_CURVE:
					{
						break;
					};

					case NK_COMMAND_RECT:
					{
						const nk_command_rect* cmdRect = reinterpret_cast<const nk_command_rect*>(cmd);

						renderer2d.AddLineRect(
							RpgRectFloat(cmdRect->x, cmdRect->y, cmdRect->x + cmdRect->w, cmdRect->y + cmdRect->h),
							RpgColor(cmdRect->color.r, cmdRect->color.g, cmdRect->color.b, cmdRect->color.a)
						);

						break;
					};

					case NK_COMMAND_RECT_FILLED:
					{
						const nk_command_rect_filled* cmdRectFilled = reinterpret_cast<const nk_command_rect_filled*>(cmd);

						renderer2d.AddMeshRect(
							RpgRectFloat(cmdRectFilled->x, cmdRectFilled->y, cmdRectFilled->x + cmdRectFilled->w, cmdRectFilled->y + cmdRectFilled->h),
							RpgColor(cmdRectFilled->color.r, cmdRectFilled->color.g, cmdRectFilled->color.b, cmdRectFilled->color.a)
						);

						break;
					};

					case NK_COMMAND_RECT_MULTI_COLOR:
					{
						break;
					};

					case NK_COMMAND_CIRCLE:
					{
						break;
					};

					case NK_COMMAND_CIRCLE_FILLED:
					{
						break;
					};

					case NK_COMMAND_ARC:
					{
						break;
					};

					case NK_COMMAND_ARC_FILLED:
					{
						break;
					};

					case NK_COMMAND_TRIANGLE:
					{
						break;
					};

					case NK_COMMAND_TRIANGLE_FILLED:
					{
						break;
					};

					case NK_COMMAND_POLYGON:
					{
						break;
					};

					case NK_COMMAND_POLYGON_FILLED:
					{
						break;
					};

					case NK_COMMAND_POLYLINE:
					{
						break;
					};

					case NK_COMMAND_TEXT:
					{
						const nk_command_text* cmdText = reinterpret_cast<const nk_command_text*>(cmd);

						renderer2d.AddText(cmdText->string, cmdText->length, RpgPointFloat(cmdText->x, cmdText->y), RpgColor::WHITE);

						break;
					};

					case NK_COMMAND_IMAGE:
					{
						break;
					};

					case NK_COMMAND_CUSTOM:
					{
						break;
					};

					default: break;
				}
			};

			nk_clear(&NkContext);
			*/

		
		}
		MainRenderer->EndRender(frameIndex, deltaTime);
	}
	RpgRenderThread::ExecuteFrame(frameCounter, frameIndex, deltaTime, MainRenderer.Get());


	// End frame
	MainWorld->EndFrame(frameIndex);

	g_InputManager->Flush();
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


void RpgEngine::SetMainCamera(RpgGameObjectID cameraObject) noexcept
{
	if (MainCameraObject == cameraObject)
	{
		return;
	}

	MainCameraObject = cameraObject;

	RpgRenderComponent_Camera* cameraComp = MainWorld->GameObject_AddComponent<RpgRenderComponent_Camera>(MainCameraObject);
	cameraComp->Viewport = &SceneViewport;
	cameraComp->bActivated = true;
}
