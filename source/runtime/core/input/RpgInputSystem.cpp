#include "RpgInputSystem.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogInput, VERBOSITY_DEBUG)



RpgInputSystem* g_InputSystem = nullptr;


RpgInputSystem::RpgInputSystem() noexcept
{
	RpgPlatformMemory::Zero(KeyButtonDown[0], sizeof(bool) * RpgInputKey::MAX_COUNT);
	RpgPlatformMemory::Zero(KeyButtonDown[1], sizeof(bool) * RpgInputKey::MAX_COUNT);
}


void RpgInputSystem::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{
	MouseCursorPosition[1] = RpgPointFloat(e.Position);
	MouseCursorDeltaPosition += RpgPointFloat(e.DeltaPosition);
}


void RpgInputSystem::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{
	MouseScrollValue = RpgPointFloat(e.ScrollValue);
}


void RpgInputSystem::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{
	const RpgInputKey::EButton mb = static_cast<RpgInputKey::EButton>(e.Button);
	KeyButtonDown[1][mb] = e.bIsDown;
}


void RpgInputSystem::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	const RpgInputKey::EButton kb = static_cast<RpgInputKey::EButton>(e.Button);
	KeyButtonDown[1][kb] = e.bIsDown;
}


void RpgInputSystem::UpdateMappingStates() noexcept
{
}


void RpgInputSystem::Flush() noexcept
{
	MouseCursorPosition[0] = MouseCursorPosition[1];
	MouseCursorDeltaPosition = RpgPointFloat();
	MouseScrollValue = RpgPointFloat();
	RpgPlatformMemory::Copy(KeyButtonDown[0], KeyButtonDown[1], sizeof(bool) * RpgInputKey::MAX_COUNT);
}
