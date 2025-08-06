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


	// Add control as candidate hovered
	// @param widget - Widget control
	inline void AddControlHovered(RpgGuiWidget* widget) noexcept
	{
		RPG_Check(widget);
		ControlHoveredCandidates.AddValue(widget);
	}


	// Add layout as a candidate hovered
	// @param layout - Widget layout
	inline void AddLayoutHovered(RpgGuiWidget* layout) noexcept
	{
		RPG_Check(layout);

		//RPG_LogDebug(RpgLogTemp, "Add candicate hovered layout (%s)", *layout->Name);
		LayoutHoveredCandidates.AddValue(layout);
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

		if (WidgetFocused != widget)
		{
			if (WidgetFocused)
			{
				WidgetFocused->FocusedExit();
			}

			widget->FocusedEnter();
			WidgetFocused = widget;
		}

		return true;
	}


	// Get current hovered layout
	// @return Current hovered layout
	inline RpgGuiWidget* GetLayoutHovered() const noexcept
	{
		return LayoutHovered;
	}


	// Get current hovered control
	// @return Current hovered control
	inline RpgGuiWidget* GetControlHovered() const noexcept
	{
		return ControlHovered;
	}

	// Get current pressed widget
	// @return Current pressed widget
	inline RpgGuiWidget* GetWidgetPressed() const noexcept
	{
		return WidgetPressed;
	}

	// Get current focused widget
	// @return Current input focused widget
	inline RpgGuiWidget* GetWidgetFocused() const noexcept
	{
		return WidgetFocused;
	}

	// Force clear focused widget
	inline void ClearWidgetFocused() noexcept
	{
		if (WidgetFocused)
		{
			WidgetFocused->FocusedExit();
		}

		WidgetFocused = nullptr;
	}


private:
	RpgArrayInline<RpgGuiWidget*, 8> LayoutHoveredCandidates;
	RpgGuiWidget* LayoutHovered;

	RpgArrayInline<RpgGuiWidget*, 4> ControlHoveredCandidates;
	RpgGuiWidget* ControlHovered;

	RpgGuiWidget* WidgetPressed;
	RpgGuiWidget* WidgetReleased;
	RpgGuiWidget* WidgetFocused;

	bool KeyButtonDown[RpgInputKey::MAX_COUNT];
	bool KeyButtonUp[RpgInputKey::MAX_COUNT];

};
