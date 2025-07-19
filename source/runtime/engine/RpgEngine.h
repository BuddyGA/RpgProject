#pragma once

#include "core/RpgString.h"
#include "core/RpgPointer.h"
#include "core/world/RpgWorld.h"
#include "render/RpgRenderer.h"
#include "render/RpgSceneViewport.h"
#include "gui/RpgGuiTypes.h"
#include "script/RpgScript_DebugCamera.h"



extern class RpgEngine* g_Engine;

class RpgEngine 
{
	RPG_NOCOPYMOVE(RpgEngine)

public:
	RpgEngine() noexcept;
	~RpgEngine() noexcept;

	void Initialize() noexcept;

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


	inline void SetMouseRelativeMode(bool bEnable) noexcept
	{
		RpgPlatformMouse::SetEnableRelativeMode(RpgPlatformProcess::GetMainWindowHandle(), bEnable);
	}

	inline void SetMouseCursorPosition(int x, int y) noexcept
	{
		RpgPlatformMouse::SetCursorPosition(RpgPlatformProcess::GetMainWindowHandle(), RpgPointInt(x, y));
	}


	inline void ClipMouseCursor(bool bClip) noexcept
	{
		if (bClip)
		{
			RpgRectInt rect;
			rect.Left = 0;
			rect.Top = 0;
			rect.Right = WindowDimension.X - 2;
			rect.Bottom = WindowDimension.Y - 2;

			RpgPlatformMouse::SetEnableClipCursor(RpgPlatformProcess::GetMainWindowHandle(), rect);
		}
		else
		{
			RpgPlatformMouse::SetDisableClipCursor();
		}
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
	RpgUniquePtr<RpgGuiCanvas> GuiCanvas;

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
