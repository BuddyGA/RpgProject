#pragma once

#include "RpgInputTypes.h"


class RpgInputManager
{
	RPG_NOCOPYMOVE(RpgInputManager)

public:
	RpgInputManager() noexcept;

	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;

	void UpdateMappingStates() noexcept;

	// Record current frame key states as previous frame. Called by RpgGameApp at the end of FrameTick
	void Flush() noexcept;


	// Get current mouse cursor position
	// @returns Point (X,Y) in window client
	inline RpgPointFloat GetMouseCursorPosition() const noexcept
	{
		return MouseCursorPosition[1];
	}

	// Get mouse cursor delta position between current and previous frame
	// @returns Delta in (X,Y) axis
	inline RpgPointFloat GetMouseCursorDeltaPosition() const noexcept
	{
		return MouseCursorDeltaPosition;
	}

	// Get current mouse scroll/wheel value
	// @returns Value in (X,Y) axis
	inline RpgPointFloat GetMouseScrollValue() const noexcept
	{
		return MouseScrollValue;
	}

	// Get current key button state
	// @param key - Key button
	// @returns Button state
	inline RpgInputButtonState GetKeyButtonState(RpgInputKey::EButton button) const noexcept
	{
		if (!KeyButtonDown[0][button] && KeyButtonDown[1][button])
		{
			return RpgInputButtonState::PRESSED;
		}

		if (KeyButtonDown[0][button] && KeyButtonDown[1][button])
		{
			return RpgInputButtonState::DOWN;
		}

		if (KeyButtonDown[0][button] && !KeyButtonDown[1][button])
		{
			return RpgInputButtonState::RELEASED;
		}

		return RpgInputButtonState::NONE;
	}

	// Is key button pressed?
	// @param key - Key button
	// @returns TRUE if key button pressed this frame
	inline bool IsKeyButtonPressed(RpgInputKey::EButton button) const noexcept
	{
		return GetKeyButtonState(button) == RpgInputButtonState::PRESSED;
	}

	// Is key button held down?
	// @param key - Key button
	// @returns TRUE if key button held down this frame
	inline bool IsKeyButtonDown(RpgInputKey::EButton button) const noexcept
	{
		return GetKeyButtonState(button) == RpgInputButtonState::PRESSED || GetKeyButtonState(button) == RpgInputButtonState::DOWN;
	}

	// Is key button released?
	// @param key - Key button
	// @returns TRUE if key button released this frame
	inline bool IsKeyButtonReleased(RpgInputKey::EButton button) const noexcept
	{
		return GetKeyButtonState(button) == RpgInputButtonState::RELEASED;
	}


private:
	struct FKeyButtonState
	{
		bool bIsDown{ false };
		bool bIsRepeat{ false };
	};


private:
	// Mouse cursor position (prev and current frame)
	RpgPointFloat MouseCursorPosition[2];

	// Mouse cursor delta position
	RpgPointFloat MouseCursorDeltaPosition;

	// Mouse scroll/wheel value
	RpgPointFloat MouseScrollValue;

	// Key button states
	// [0]: Previous, [1]: Current
	bool KeyButtonDown[2][RpgInputKey::MAX_COUNT];
};


extern RpgInputManager* g_InputManager;
