#pragma once

#include "core/RpgConsoleSystem.h"
#include "core/RpgPointer.h"
#include "world/RpgWorld.h"
#include "render/RpgSceneViewport.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogGame);


class RpgRenderer;


extern class RpgGameApp* g_GameApp;

class RpgGameApp 
{
	RPG_NOCOPYMOVE(RpgGameApp)

public:
	RpgGameApp() noexcept;
	~RpgGameApp() noexcept;

	void RequestExit(bool bAskConfirmation) noexcept;
	void HandleConsoleCommand(const RpgName& command, const RpgConsoleCommandParams& params) noexcept;
	void WindowSizeChanged(const RpgPlatformWindowEvent& e) noexcept;
	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;
	void CharInput(char c) noexcept;

	void FrameTick(int frameIndex, float deltaTime) noexcept;
	void FramePreRender(int frameIndex, float deltaTime, RpgRenderer& renderer) noexcept;


	inline bool IsWindowMinimized() const noexcept
	{
		return WindowState == RpgPlatformWindowSizeState::MINIMIZED;
	}

	inline RpgPointInt GetWindowDimension() const noexcept
	{
		return WindowDimension;
	}


private:
	// Main window size
	RpgPointInt WindowDimension;

	// Main window state
	RpgPlatformWindowSizeState WindowState;

	// Main world
	RpgWorld MainWorld;

	// Main viewport
	RpgSceneViewport MainViewport;


public:
	int FpsLimit;

private:
	float FpsSampleTimer;
	int FpsSampleFrameCount;
	float FpsTimeMs;
	float FpsCountMs;
	RpgString FpsString;

};
