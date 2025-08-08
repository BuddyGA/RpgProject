#include "RpgInputManager.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogInput, VERBOSITY_DEBUG)



RpgInputManager* g_InputManager = nullptr;


RpgInputManager::RpgInputManager() noexcept
{
	RpgPlatformMemory::MemZero(KeyButtonDown[0], sizeof(bool) * RpgInputKey::MAX_COUNT);
	RpgPlatformMemory::MemZero(KeyButtonDown[1], sizeof(bool) * RpgInputKey::MAX_COUNT);
}


void RpgInputManager::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	MouseCursorPosition[1] = RpgPointFloat(e.Position);
	MouseCursorDeltaPosition += RpgPointFloat(e.DeltaPosition);
}


void RpgInputManager::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	MouseScrollValue = RpgPointFloat(e.ScrollValue);
}


void RpgInputManager::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	const RpgInputKey::EButton mb = static_cast<RpgInputKey::EButton>(e.Button);
	KeyButtonDown[1][mb] = e.bIsDown;
}


void RpgInputManager::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	const RpgInputKey::EButton kb = static_cast<RpgInputKey::EButton>(e.KeyCode);
	KeyButtonDown[1][kb] = e.bIsDown;
}


void RpgInputManager::UpdateMappingStates() noexcept
{
}


void RpgInputManager::Flush() noexcept
{
	MouseCursorPosition[0] = MouseCursorPosition[1];
	MouseCursorDeltaPosition = RpgPointFloat();
	MouseScrollValue = RpgPointFloat();
	RpgPlatformMemory::MemCopy(KeyButtonDown[0], KeyButtonDown[1], sizeof(bool) * RpgInputKey::MAX_COUNT);
}
