#pragma once

#include "core/RpgConsoleSystem.h"
#include "core/RpgPointer.h"
#include "core/world/RpgWorld.h"
#include "render/RpgRenderer.h"
#include "render/RpgSceneViewport.h"
#include "gui/RpgGuiContext.h"
#include "gui/RpgGuiCanvas.h"
#include "gui/widget/RpgGuiConsole.h"
#include "script/RpgScript_DebugCamera.h"



extern class RpgEngine* g_Engine;

class RpgEngine 
{
	RPG_NOCOPYMOVE(RpgEngine)

public:
	RpgEngine() noexcept;
	~RpgEngine() noexcept;

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

	[[nodiscard]] RpgWorld* CreateWorld(const RpgName& name) noexcept;
	void DestroyWorld(RpgWorld*& world) noexcept;

	void SetMainCamera(RpgGameObjectID cameraObject) noexcept;


	inline bool IsWindowMinimized() const noexcept
	{
		return WindowState == RpgPlatformWindowSizeState::MINIMIZED;
	}


	inline RpgWorld* GetMainWorld() noexcept
	{
		return MainWorld;
	}

	inline const RpgWorld* GetMainWorld() const noexcept
	{
		return MainWorld;
	}


private:
	void CreateTestLevel() noexcept;


private:
	// Main window size
	RpgPointInt WindowDimension;

	// Main window state
	RpgPlatformWindowSizeState WindowState;


	// Created worlds. Main world always at index 0
	RpgArray<RpgUniquePtr<RpgWorld>> Worlds;
	RpgWorld* MainWorld;

	// Main renderer
	RpgUniquePtr<RpgRenderer> MainRenderer;

	// Main scene viewport
	RpgSceneViewport SceneViewport;

	// GUI context
	RpgGuiContext GuiContext;
	RpgGuiCanvas GuiCanvas;

	// GUI console
	RpgGuiConsole* GuiConsole;

	// Main camera object inside main world
	RpgGameObjectID MainCameraObject;

	// Script camera
	RpgScript_DebugCamera ScriptDebugCamera;


public:
	int FpsLimit;

private:
	float FpsSampleTimer;
	int FpsSampleFrameCount;
	float FpsTimeMs;
	float FpsCountMs;
	RpgString FpsString;

};
