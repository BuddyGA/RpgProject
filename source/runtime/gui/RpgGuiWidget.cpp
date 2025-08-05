#include "RpgGuiWidget.h"
#include "RpgGuiContext.h"
#include "render/RpgRenderer2D.h"



RpgGuiWidget::RpgGuiWidget() noexcept
{
	Flags = RpgGui::FLAG_None;
	Order = 255;
}


RpgGuiWidget::RpgGuiWidget(const RpgName& in_Name) noexcept
	: RpgGuiWidget()
{
	Name = in_Name;
}


RpgRectFloat RpgGuiWidget::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
{
	if (!IsVisible())
	{
		return RpgRectFloat();
	}

	AbsoluteRect = CalculateAbsoluteRect(offset);

	for (int c = 0; c < Children.GetCount(); ++c)
	{
		Children[c]->UpdateRect(context, canvasRect, AbsoluteRect.GetPosition());
	}

	return AbsoluteRect;
}


void RpgGuiWidget::UpdateState(RpgGuiContext& context) noexcept
{
	if (!IsVisible())
	{
		return;
	}

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

	if (bWasFocused && (context.CurrentFocused != this || (bMouseLeftPressed && !bIsMouseCursorInside)))
	{
		RPG_LogDebug(RpgLogGui, "%s focused (exit)", *Name);
		Flags &= ~RpgGui::FLAG_State_Focused;
		OnFocusedExit(context);

		if (context.CurrentFocused == this)
		{
			context.CurrentFocused = nullptr;
		}
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
		context.AddHoveredLayout(this);
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
		const bool bHovered = bIsLayout ? (context.CurrentHoveredLayout == this) : (context.CurrentHovered == this);

		if (bHovered)
		{
			RPG_LogDebug(RpgLogGui, "%s hovered (enter) (order: %u)", *Name, Order);
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


void RpgGuiWidget::Render(const RpgGuiContext& context, RpgRenderer2D& renderer, uint8_t parentOrder, const RpgRectFloat& parentClipRect) const noexcept
{
	if (!(IsVisible() && parentClipRect.IsRectIntersect(AbsoluteRect)))
	{
		return;
	}

	if (IsLayout())
	{
		RPG_Check(Order != 255);

		//RPG_LogDebug(RpgLogGui, "(Name: %s, Order: %u)", *Name, Order);

		renderer.SetOrder(Order);
		renderer.SetClipRect(RpgRectInt(parentClipRect));

		// Render self
		OnRender(renderer);

		// Set clip rect for children
		const RpgRectFloat childrenClipRect(
			RpgMath::Max(AbsoluteRect.Left, parentClipRect.Left),
			RpgMath::Max(AbsoluteRect.Top, parentClipRect.Top),
			RpgMath::Min(AbsoluteRect.Right, parentClipRect.Right),
			RpgMath::Min(AbsoluteRect.Bottom, parentClipRect.Bottom)
		);

		renderer.SetClipRect(RpgRectInt(childrenClipRect));

		for (int c = 0; c < Children.GetCount(); ++c)
		{
			Children[c]->Render(context, renderer, Order, childrenClipRect);
		}

		//RPG_LogDebug(RpgLogGui, "(Name: %s, ParentOrder: %u)", *Name, parentOrder);
		// Set order and clip back to parent
		renderer.SetOrder(parentOrder);
		renderer.SetClipRect(RpgRectInt(parentClipRect));
	}
	else
	{
		RPG_Check(Children.IsEmpty());

		OnRender(renderer);
	}
}
