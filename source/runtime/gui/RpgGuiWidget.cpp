#include "RpgGuiWidget.h"
#include "RpgGuiContext.h"
#include "render/RpgRenderer2D.h"



RpgGuiWidget::RpgGuiWidget() noexcept
{
	Flags = RpgGui::FLAG_None;
	BackgroundColor = RpgColor(50, 50, 50);
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


void RpgGuiWidget::UpdateState(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept
{
	if (!IsVisible())
	{
		return;
	}

	const bool bIsLayout = IsLayout();

	// Update self
	OnUpdate(context, parentLayout);

	// Check if cursor intersect with rect
	if (AbsoluteRect.IsPointInside(context.MouseCursorPosition))
	{
		if (bIsLayout)
		{
			context.AddLayoutHovered(this);
		}
		else if (parentLayout->IsHovered())
		{
			RPG_Check(Children.IsEmpty());
			context.AddControlHovered(this);
		}
	}
	else
	{
		HoveredExit();
	}
	
	// Propagate update state to children
	for (int c = 0; c < Children.GetCount(); ++c)
	{
		Children[c]->UpdateState(context, this);
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


void RpgGuiWidget::OnRender(RpgRenderer2D& renderer) const noexcept
{
	if (BackgroundColor.A > 0)
	{
		renderer.AddMeshRect(AbsoluteRect, BackgroundColor);
	}

	if (IsHovered())
	{
		if (IsLayout())
		{
			renderer.AddLineRect(AbsoluteRect, RpgColor::WHITE);
		}
		else
		{
			renderer.AddLineRect(AbsoluteRect, RpgColor::GREEN);
		}
	}
}
