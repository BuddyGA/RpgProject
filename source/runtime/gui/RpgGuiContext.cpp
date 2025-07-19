#include "RpgGuiTypes.h"



RpgGuiContext::RpgGuiContext() noexcept
{
	ScrollSpeed = 8.0f;
	TextInputChar = '\0';
	PrevHovered = nullptr;
	PrevHoveredLayout = nullptr;
	PrevPressed = nullptr;
	CurrentHovered = nullptr;
	CurrentHoveredLayout = nullptr;
	CurrentPressed = nullptr;
	CurrentFocused = nullptr;

	RpgPlatformMemory::MemZero(KeyButtonDown, sizeof(bool) * RpgInputKey::MAX_COUNT);
	RpgPlatformMemory::MemZero(KeyButtonUp, sizeof(bool) * RpgInputKey::MAX_COUNT);
}


void RpgGuiContext::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	MouseCursorPosition = RpgPointFloat(e.Position);
	MouseCursorDeltaPosition = RpgPointFloat(e.DeltaPosition);
}


void RpgGuiContext::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	MouseScrollValue = RpgPointFloat(e.ScrollValue);
}


void RpgGuiContext::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	const RpgInputKey::EButton mb = static_cast<RpgInputKey::EButton>(e.Button);

	if (e.bIsDown)
	{
		KeyButtonDown[mb] = true;
	}
	else
	{
		KeyButtonUp[mb] = true;
	}
}


void RpgGuiContext::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	const RpgInputKey::EButton kb = static_cast<RpgInputKey::EButton>(e.Button);

	if (e.bIsDown)
	{
		KeyButtonDown[kb] = true;
	}
	else
	{
		KeyButtonUp[kb] = true;
	}
}


void RpgGuiContext::CharInput(char c) noexcept
{
	TextInputChar = c;
}


void RpgGuiContext::Begin() noexcept
{
	PrevHovered = nullptr;
	PrevHoveredLayout = nullptr;
	PrevPressed = nullptr;
}


void RpgGuiContext::End() noexcept
{
	MouseCursorDeltaPosition = RpgPointFloat();
	MouseScrollValue = RpgPointFloat();
	TextInputChar = '\0';

	CurrentHovered = PrevHovered;
	CurrentHoveredLayout = PrevHoveredLayout;
	CurrentPressed = PrevPressed;

	RpgPlatformMemory::MemZero(KeyButtonDown, sizeof(bool) * RpgInputKey::MAX_COUNT);
	RpgPlatformMemory::MemZero(KeyButtonUp, sizeof(bool) * RpgInputKey::MAX_COUNT);
}


void RpgGuiContext::BeginTextInput()
{
	/*
	RPG_Assert(Window);

	RPG_LogDebug(RpgLogGui, "RpgGuiContext: Begin text input!");
	SDL_StartTextInput(Window);
	*/
}


void RpgGuiContext::EndTextInput()
{
	/*
	RPG_Assert(Window);

	RPG_LogDebug(RpgLogGui, "RpgGuiContext: End text input!");
	SDL_StopTextInput(Window);
	*/
}
