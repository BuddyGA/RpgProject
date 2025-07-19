#include "RpgGuiTypes.h"
#include "input/RpgInputManager.h"
#include "render/RpgRenderer2D.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogGui, VERBOSITY_DEBUG)



RpgPointFloat RpgGui::CalculateTextAlignmentPosition(const RpgRectFloat& rect, const RpgString& text, const RpgSharedFont& font, RpgGuiAlignHorizontal alignH, RpgGuiAlignVertical alignV) noexcept
{
	const RpgPointFloat rectDim = rect.GetDimension();
	const RpgPointFloat textDim = font->CalculateTextDimension(*text, text.GetLength());

	RpgPointFloat pos;

	switch (alignH)
	{
		case RpgGuiAlignHorizontal::LEFT: pos.X = rect.Left; break;
		case RpgGuiAlignHorizontal::CENTER: pos.X = rect.Left + (rectDim.X - textDim.X) * 0.5f; break;
		case RpgGuiAlignHorizontal::RIGHT: pos.X = rect.Right - textDim.X; break;
		default: break;
	}

	switch (alignV)
	{
		case RpgGuiAlignVertical::TOP: pos.Y = rect.Top; break;
		case RpgGuiAlignVertical::CENTER: pos.Y = rect.Top + (rectDim.Y - textDim.Y) * 0.5f; break;
		case RpgGuiAlignVertical::BOTTOM: pos.Y = rect.Bottom - textDim.Y; break;
		default: break;
	}

	return pos;
}



RpgGuiWidget::RpgGuiWidget() noexcept
{
	Flags = RpgGui::FLAG_None;
}


RpgRectFloat RpgGuiWidget::UpdateRect(const RpgGuiContext& context, const RpgPointFloat& offset) noexcept
{
	AbsoluteRect = CalculateAbsoluteRect(offset);

	for (int c = 0; c < Children.GetCount(); ++c)
	{
		Children[c]->UpdateRect(context, AbsoluteRect.GetPosition());
	}

	return AbsoluteRect;
}


void RpgGuiWidget::UpdateState(RpgGuiContext& context) noexcept
{
	OnUpdate(context);

	const bool bMouseLeftPressed = context.IsKeyButtonDown(RpgInputKey::MOUSE_LEFT);
	const bool bMouseLeftReleased = context.IsKeyButtonUp(RpgInputKey::MOUSE_LEFT);
	const bool bIsMouseCursorInside = AbsoluteRect.IsPointInside(context.MouseCursorPosition);
	const bool bIsLayout = IsLayout();
	const bool bWasHovered = IsHovered();
	const bool bWasPressed = IsPressed();
	const bool bWasReleased = IsReleased();
	const bool bWasFocused = IsFocused();

	if (bIsLayout && bWasHovered && context.CurrentHoveredLayout != this)
	{
		RPG_LogDebug(RpgLogGui, "%s hovered (exit)", *Name);
		Flags &= ~RpgGui::FLAG_State_Hovered;
		OnHoveredExit(context);
	}

	// Remove pressed state if mouse left released even if cursor is outside rect
	if (bWasPressed && bMouseLeftReleased)
	{
		Flags &= ~RpgGui::FLAG_State_Pressed;
	}

	if (bWasReleased)
	{
		Flags &= ~RpgGui::FLAG_State_Released;
	}

	if (bWasFocused && (context.CurrentFocused != this || (bMouseLeftPressed && !bIsMouseCursorInside)) )
	{
		RPG_LogDebug(RpgLogGui, "%s focused (exit)", *Name);
		Flags &= ~RpgGui::FLAG_State_Focused;
		OnFocusedExit(context);
	}

	// Check if cursor intersect with rect
	if (!bIsMouseCursorInside)
	{
		if (bWasHovered)
		{
			RPG_LogDebug(RpgLogGui, "%s hovered (exit)", *Name);
			Flags &= ~RpgGui::FLAG_State_Hovered;
			OnHoveredExit(context);
		}

		// Propagate update state to children
		for (int c = 0; c < Children.GetCount(); ++c)
		{
			Children[c]->UpdateState(context);
		}

		return;
	}


	if (bIsLayout)
	{
		context.PrevHoveredLayout = this;
	}
	else
	{
		context.PrevHovered = this;
	}

	if (bWasHovered)
	{
		if (bMouseLeftPressed)
		{
			context.PrevPressed = this;
		}

		// If was released while mouse cursor hovering
		if (bWasReleased)
		{
			// Check if can receive focus
			if (!bWasFocused && HasInputFocus())
			{
				context.CurrentFocused = this;

				RPG_LogDebug(RpgLogGui, "%s focused (enter)", *Name);
				Flags |= RpgGui::FLAG_State_Focused;
				OnFocusedEnter(context);
			}
		}
		// If was pressed while mouse cursor hovering
		else if (bWasPressed)
		{
			// If mouse left released while mouse cursor hovering
			if (bMouseLeftReleased)
			{
				RPG_LogDebug(RpgLogGui, "%s released", *Name);
				Flags |= RpgGui::FLAG_State_Released;
				OnReleased(context);
			}
		}
		else if (context.CurrentPressed == this)
		{
			RPG_LogDebug(RpgLogGui, "%s pressed", *Name);
			Flags |= RpgGui::FLAG_State_Pressed;
			OnPressed(context);
		}
	}
	else
	{
		const bool bCurrentHovered = bIsLayout ? (context.CurrentHoveredLayout == this) : (context.CurrentHovered == this);

		if (bCurrentHovered)
		{
			RPG_LogDebug(RpgLogGui, "%s hovered (enter)", *Name);
			Flags |= RpgGui::FLAG_State_Hovered;
			OnHoveredEnter(context);
		}
	}

	// Propagate update state to children
	for (int c = 0; c < Children.GetCount(); ++c)
	{
		Children[c]->UpdateState(context);
	}
}


void RpgGuiWidget::Render(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept
{
	OnRender(context, renderer, parentClipRect);

	const bool bHasClipRect = IsLayout();
	RpgRectFloat clipRect = parentClipRect;

	if (bHasClipRect)
	{
		clipRect.Left = RpgMath::Max(AbsoluteRect.Left, parentClipRect.Left);
		clipRect.Top = RpgMath::Max(AbsoluteRect.Top, parentClipRect.Top);
		clipRect.Right = RpgMath::Min(AbsoluteRect.Right, parentClipRect.Right);
		clipRect.Bottom = RpgMath::Min(AbsoluteRect.Bottom, parentClipRect.Bottom);
		renderer.PushClipRect(RpgRectInt(clipRect));
	}

	for (int c = 0; c < Children.GetCount(); ++c)
	{
		Children[c]->Render(context, renderer, clipRect);
	}

	if (bHasClipRect)
	{
		renderer.PopClipRect();
	}
}
