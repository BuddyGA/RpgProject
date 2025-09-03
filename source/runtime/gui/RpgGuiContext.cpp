#include "RpgGuiContext.h"



RpgGuiContext::RpgGuiContext() noexcept
{
	ScrollSpeed = 8.0f;
	TextInputChar = '\0';
	LayoutHovered = nullptr;
	ControlHovered = nullptr;
	WidgetPressed = nullptr;
	WidgetReleased = nullptr;
	WidgetFocused = nullptr;

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
	RpgGuiWidget* prevLayoutHovered = LayoutHovered;
	LayoutHovered = nullptr;

	for (int i = 0; i < LayoutHoveredCandidates.GetCount(); ++i)
	{
		RpgGuiWidget* layout = LayoutHoveredCandidates[i];
		RPG_Check(layout->IsLayout());

		if (LayoutHovered == nullptr || layout->Order <= LayoutHovered->Order)
		{
			LayoutHovered = layout;
		}
	}

	if (prevLayoutHovered && prevLayoutHovered != LayoutHovered)
	{
		prevLayoutHovered->HoveredExit();
	}

	if (LayoutHovered)
	{
		LayoutHovered->HoveredEnter();
	}

	RpgGuiWidget* prevControlHovered = ControlHovered;
	ControlHovered = nullptr;

	for (int i = 0; i < ControlHoveredCandidates.GetCount(); ++i)
	{
		RpgGuiWidget* control = ControlHoveredCandidates[i];
		RPG_Check(!control->IsLayout() && control->Children.IsEmpty());

		if (ControlHovered == nullptr || control->Order <= ControlHovered->Order)
		{
			ControlHovered = control;
		}
	}

	if (prevControlHovered && prevControlHovered != ControlHovered)
	{
		prevControlHovered->HoveredExit();
	}

	if (ControlHovered)
	{
		ControlHovered->HoveredEnter();
	}

	const bool bMouseLeftDown = IsKeyButtonDown(RpgInputKey::MOUSE_LEFT);
	const bool bMouseLeftUp = IsKeyButtonUp(RpgInputKey::MOUSE_LEFT);
	
	// current hovered
	RpgGuiWidget* hoveredWidget = ControlHovered ? ControlHovered : LayoutHovered;
	if (hoveredWidget)
	{
		RPG_Check(hoveredWidget->IsHovered());
		const bool bWasPressed = (WidgetPressed == hoveredWidget);
		const bool bWasReleased = (WidgetReleased == hoveredWidget);

		if (bWasReleased)
		{
			RPG_Check(hoveredWidget->Flags & RpgGui::FLAG_State_Released);
			hoveredWidget->Flags &= ~RpgGui::FLAG_State_Released;
			WidgetReleased = nullptr;

			if (hoveredWidget->HasInputFocus())
			{
				hoveredWidget->FocusedEnter();
				WidgetFocused = hoveredWidget;
			}
		}
		else if (bWasPressed)
		{
			if (bMouseLeftUp)
			{
				WidgetPressed = nullptr;

				hoveredWidget->Released();
				WidgetReleased = hoveredWidget;
			}
		}
		else if (bMouseLeftDown)
		{
			hoveredWidget->Pressed();
			WidgetPressed = hoveredWidget;

			if (WidgetFocused && WidgetFocused != hoveredWidget)
			{
				WidgetFocused->FocusedExit();
				WidgetFocused = nullptr;
			}
		}
	}

	// if we have previouly pressed widget and mouse-left-button up while hovering outside of it
	if (WidgetPressed && WidgetPressed != hoveredWidget && bMouseLeftUp)
	{
		RPG_Check(WidgetPressed->Flags & RpgGui::FLAG_State_Pressed);
		WidgetPressed->Flags &= ~RpgGui::FLAG_State_Pressed;
		WidgetPressed = nullptr;
	}
	

	//RPG_Log(RpgLogGui, "HoveredLayout: %s", HoveredLayout ? *HoveredLayout->Name : "NONE");

	LayoutHoveredCandidates.Clear();
	ControlHoveredCandidates.Clear();
}


void RpgGuiContext::End() noexcept
{
	MouseCursorDeltaPosition = RpgPointFloat();
	MouseScrollValue = RpgPointFloat();
	TextInputChar = '\0';

	RpgPlatformMemory::MemZero(KeyButtonDown, sizeof(bool) * RpgInputKey::MAX_COUNT);
	RpgPlatformMemory::MemZero(KeyButtonUp, sizeof(bool) * RpgInputKey::MAX_COUNT);
}
