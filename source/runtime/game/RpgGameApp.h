#pragma once

#include "core/RpgConsoleSystem.h"
#include "core/RpgPointer.h"
#include "render/RpgRenderer.h"
#include "render/RpgSceneViewport.h"
#include "gui/RpgGuiContext.h"
#include "gui/RpgGuiCanvas.h"
#include "gui/widget/RpgGuiConsole.h"
#include "world/RpgGameWorld.h"



RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogGame);



extern class RpgGameApp* g_GameApp;

class RpgGameApp 
{
	RPG_NOCOPYMOVE(RpgGameApp)

public:
	RpgGameApp() noexcept;
	~RpgGameApp() noexcept;

	void Initialize() noexcept;
	void HandleConsoleCommand(const RpgName& command, const RpgConsoleCommandParams& params) noexcept;
	void WindowSizeChanged(const RpgPlatformWindowEvent& e) noexcept;
	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;
	void CharInput(char c) noexcept;

	void FrameTick(uint64_t frameCounter, float deltaTime) noexcept;
	void RequestExit(bool bAskConfirmation) noexcept;

	void OpenLevel(const RpgString& levelAssetPath) noexcept;
	void SetMainCamera(RpgGameObject cameraObject) noexcept;


	inline bool IsWindowMinimized() const noexcept
	{
		return WindowState == RpgPlatformWindowSizeState::MINIMIZED;
	}

	inline RpgPointInt GetWindowDimension() const noexcept
	{
		return WindowDimension;
	}


	template<typename TWorld>
	inline TWorld* CreateWorld(const RpgName& name) noexcept
	{
		static_assert(std::is_base_of<RpgWorld, TWorld>::value, "RpgGameApp::CreateWorld type of <TWorld> must be derived from type <RpgWorld>!");

		const int index = Worlds.GetCount();
		Worlds.AddValue(RpgPointer::MakeUnique<TWorld>(name));

		TWorld* world = static_cast<TWorld*>(Worlds[index].Get());
		world->Initialize();

		return world;
	}


	void DestroyWorld(RpgWorld* world) noexcept
	{
		RPG_Check(world);

		const int index = Worlds.FindIndexByCompare(world);
		if (index != RPG_INDEX_INVALID)
		{
			Worlds.RemoveAt(index);
			world = nullptr;

			return;
		}

		RPG_LogWarn(RpgLogGame, "Fail to destroy world. World (%s) not found!", *world->GetName());
	}


	inline RpgWorld* GetMainWorld() noexcept
	{
		return MainWorld;
	}

	inline const RpgWorld* GetMainWorld() const noexcept
	{
		return MainWorld;
	}

	inline RpgRenderer* GetMainRenderer() noexcept
	{
		return MainRenderer.Get();
	}

	inline const RpgRenderer* GetMainRenderer() const noexcept
	{
		return MainRenderer.Get();
	}

	inline RpgSceneViewport& GetMainSceneViewport() noexcept
	{
		return SceneViewport;
	}

	inline const RpgSceneViewport& GetMainSceneViewport() const noexcept
	{
		return SceneViewport;
	}


private:
	// Main window size
	RpgPointInt WindowDimension;

	// Main window state
	RpgPlatformWindowSizeState WindowState;

	// Created worlds. Main world always at index 0
	RpgArray<RpgUniquePtr<RpgWorld>> Worlds;
	RpgGameWorld* MainWorld;

	// Main renderer
	RpgUniquePtr<RpgRenderer> MainRenderer;

	// Main scene viewport
	RpgSceneViewport SceneViewport;

	// GUI context
	RpgGuiContext GuiContext;
	RpgGuiCanvas GuiCanvas;

	// GUI console
	RpgGuiConsole* GuiConsole;

	// Main camera object
	RpgGameObject MainCameraObject;

	RpgLevel* LoadingLevel;


public:
	int FpsLimit;

private:
	float FpsSampleTimer;
	int FpsSampleFrameCount;
	float FpsTimeMs;
	float FpsCountMs;
	RpgString FpsString;

};
