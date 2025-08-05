#pragma once

#include "RpgGuiWidget.h"



class RpgGuiContext
{
public:
	RpgPointFloat MouseCursorPosition;
	RpgPointFloat MouseCursorDeltaPosition;
	RpgPointFloat MouseScrollValue;
	float ScrollSpeed;
	char TextInputChar;

	RpgGuiWidget* PrevHovered;
	//RpgGuiWidget* PrevHoveredLayout;
	RpgGuiWidget* PrevPressed;
	RpgGuiWidget* CurrentHovered;
	RpgGuiWidget* CurrentHoveredLayout;
	RpgGuiWidget* CurrentPressed;
	RpgGuiWidget* CurrentFocused;


public:
	RpgGuiContext() noexcept;


	// Called by RpgEngine when receiving mouse move event
	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;

	// Called by RpgEngine when receiving mouse wheel event
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;

	// Called by RpgEngine when receiving mouse button event
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;

	// Called by RpgEngine when receiving keyboard button event
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;

	// Called by RpgEngine when receiving char input
	void CharInput(char c) noexcept;

	// Called by RpgEngine to begin GUI drawing
	void Begin() noexcept;

	// Called by RpgEngine to end GUI drawing
	void End() noexcept;

	// Check if any mouse button down/pressed this frame
	// @return TRUE if any mouse button down
	inline bool IsMouseButtonAnyDown() const noexcept
	{
		return IsKeyButtonDown(RpgInputKey::MOUSE_LEFT) || IsKeyButtonDown(RpgInputKey::MOUSE_MIDDLE) || IsKeyButtonDown(RpgInputKey::MOUSE_RIGHT);
	}

	// Check if key button down/pressed this frame
	// @param button - Input key button
	// @return TRUE if key button down
	inline bool IsKeyButtonDown(RpgInputKey::EButton button) const noexcept
	{
		return KeyButtonDown[button];
	}

	// Check if key button up/released this frame
	// @param button - Input key button
	// @return TRUE if key button up/released
	inline bool IsKeyButtonUp(RpgInputKey::EButton button) const noexcept
	{
		return KeyButtonUp[button];
	}

	// Add layout as a candidate/potential hovered
	// @param layout - Widget layout
	inline void AddHoveredLayout(RpgGuiWidget* layout) noexcept
	{
		RPG_Check(layout);
		CandidateHoveredLayouts.AddValue(layout);

		/*
		if (PrevHoveredLayout == nullptr || layout->Order <= PrevHoveredLayout->Order)
		{
			PrevHoveredLayout = layout;
		}
		*/
	}

	// Force set the current focused widget
	// @param widget - Widget to receive input focus
	// @return TRUE if widget can receive input focus
	inline bool SetFocusWidget(RpgGuiWidget* widget) noexcept
	{
		RPG_Check(widget);

		if (!widget->HasInputFocus())
		{
			return false;
		}

		if (CurrentFocused != widget)
		{
			widget->Flags |= RpgGui::FLAG_State_Focused;
			widget->OnFocusedEnter(*this);
			CurrentFocused = widget;
		}

		return true;
	}


private:
	RpgArrayInline<RpgGuiWidget*, 8> CandidateHoveredLayouts;
	RpgArrayInline<RpgGuiWidget*, 4> CandicateHoveredControls;

	bool KeyButtonDown[RpgInputKey::MAX_COUNT];
	bool KeyButtonUp[RpgInputKey::MAX_COUNT];

};
